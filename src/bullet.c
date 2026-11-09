#include <stdlib.h>
#include <unistd.h>

#include "bullet.h"
#include "game.h"
#include "ui.h"

// Apelata cu board_lock luat, cand proiectilul dispare.
static void bullet_finish_locked(void)
{
    game.active_bullets--;
    if (game.active_bullets == 0)
        pthread_cond_signal(&game.bullets_done);
}

// Apelata cu board_lock luat.
static void bullet_hit_tank_locked(tank_t *target, int shooter)
{
    if (target->lives > 0)
        target->lives--;
    ui_draw_lives();

    if (target->lives == 0 && game.isRunning) {
        game.isRunning = 0;
        game.winner = shooter;
    }
}

static tank_t *tank_at_locked(position_t p)
{
    char cell = game.board[p.row][p.col];
    for (int i = 0; i < 2; i++)
        if (game.tanks[i].name == cell)
            return &game.tanks[i];
    return NULL;
}

static void *bullet_thread(void *arg)
{
    bullet_t *b = arg;

    for (;;) {
        pthread_mutex_lock(&game.board_lock);

        int done = !game.isRunning;

        if (!done) {
            position_t next = board_step(b->pos, b->dir);

            if (!board_in_bounds(next) || game.board[next.row][next.col] == CELL_WALL) {
                done = 1;
            } else {
                tank_t *target = tank_at_locked(next);
                if (target != NULL) {
                    bullet_hit_tank_locked(target, b->owner);
                    done = 1;
                } else {
                    // Pozitie libera (sau cu alt proiectil): proiectilul avanseaza.
                    if (b->on_board) {
                        game.bullets[b->pos.row][b->pos.col]--;
                        ui_draw_cell(b->pos);
                    }
                    b->pos = next;
                    b->on_board = 1;
                    game.bullets[next.row][next.col]++;
                    ui_draw_cell(next);
                }
            }
        }

        if (done) {
            if (b->on_board) {
                game.bullets[b->pos.row][b->pos.col]--;
                ui_draw_cell(b->pos);
            }
            ui_refresh();
            bullet_finish_locked();
            pthread_mutex_unlock(&game.board_lock);
            break;
        }

        ui_refresh();
        pthread_mutex_unlock(&game.board_lock);

        // Dormim fara lock, ca restul thread-urilor sa poata lucra.
        usleep(BULLET_DELAY_US);
    }

    free(b);
    return NULL;
}

void bullet_fire(const tank_t *shooter)
{
    bullet_t *b = malloc(sizeof(*b));
    if (b == NULL)
        return;

    pthread_mutex_lock(&game.board_lock);
    if (!game.isRunning) {
        pthread_mutex_unlock(&game.board_lock);
        free(b);
        return;
    }
    // Proiectilul porneste din pozitia tancului si iese in directia lui.
    b->pos = shooter->pos;
    b->dir = shooter->dir;
    b->owner = shooter->index;
    b->on_board = 0;
    game.active_bullets++;
    pthread_mutex_unlock(&game.board_lock);

    pthread_t thread;
    if (pthread_create(&thread, NULL, bullet_thread, b) != 0) {
        pthread_mutex_lock(&game.board_lock);
        bullet_finish_locked();
        pthread_mutex_unlock(&game.board_lock);
        free(b);
        return;
    }
    // Nimeni nu face join pe proiectile: resursele se elibereaza singure la final.
    pthread_detach(thread);
}
