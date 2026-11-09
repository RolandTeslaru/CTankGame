#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "tank.h"
#include "bullet.h"
#include "game.h"
#include "ui.h"

// asignez tastele tancului
// creez semaforul cu permiiusnile under doar eu pot sal accesez si, 
// O_CREAT creazxa daca nu exista si O_EXCL esueaza daca exista deja 

int tank_init(tank_t *tank, int index, char name, const char *keys,
              position_t start, direction_t dir)
{
    tank->controls.up    = tolower((unsigned char)keys[0]);
    tank->controls.down  = tolower((unsigned char)keys[1]);
    tank->controls.left  = tolower((unsigned char)keys[2]);
    tank->controls.right = tolower((unsigned char)keys[3]);
    tank->controls.fire  = tolower((unsigned char)keys[4]);

    tank->name = name;
    tank->index = index;
    tank->lives = TANK_LIVES;
    tank->pos = start;
    tank->dir = dir;

    tank->key_head = 0;
    tank->key_count = 0;
    pthread_mutex_init(&tank->key_lock, NULL);

    char sem_name[32];
    snprintf(sem_name, sizeof(sem_name), "/ctank-%d-%d", (int)getpid(), index);

    tank->keys_available = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 0);

    if (tank->keys_available == SEM_FAILED) {
        perror("sem_open");
        pthread_mutex_destroy(&tank->key_lock);
        return -1;
    }
    sem_unlink(sem_name);
    return 0;
}

void tank_destroy(tank_t *tank)
{
    sem_close(tank->keys_available);
    pthread_mutex_destroy(&tank->key_lock);
}

// Verificada daca o tasta apartine controalelor unui tank
int tank_owns_key(const tank_t *tank, int key)
{
    const controls_t *c = &tank -> controls;

    if (key == c->up || key == c->down || key == c->left || key == c->right || key == c->fire)
        return 1;
    return 0;
}


void tank_push_key(tank_t *tank, int key)
{
    pthread_mutex_lock(&tank->key_lock);
    // Daca daca coada e plina nu putem sa mai adaugam noul key
    if(tank->key_count == KEY_QUEUE_SIZE){
        pthread_mutex_unlock(&tank->key_lock);
        return;
    }
    int tail = (tank->key_head + tank->key_count) % KEY_QUEUE_SIZE; 
    tank->key_queue[tail] = key;
    tank->key_count++;
    pthread_mutex_unlock(&tank->key_lock);

    sem_post(tank->keys_available);
}

void tank_wake(tank_t *tank)
{
    sem_post(tank->keys_available);
}

// Intoarce -1 daca nu e nicio tasta (trezire doar pentru oprire).
static int tank_pop_key(tank_t *tank)
{
    int key = -1;

    pthread_mutex_lock(&tank->key_lock);
    // coada circulara de tastte (key_head + key_count) % KEY_QUEUE_SIZE
    // tancul socate de la key head, 
    if (tank->key_count > 0) {
        key = tank->key_queue[tank->key_head]; // luam cea mai veche tasta apasata din coada 
        tank->key_head = (tank->key_head + 1) % KEY_QUEUE_SIZE;
        tank->key_count --;
    }
    pthread_mutex_unlock(&tank->key_lock);
    return key;
}

static void tank_move(tank_t *tank, direction_t dir)
{
    pthread_mutex_lock(&game.board_lock);

    if (game.isRunning) {
        tank->dir = dir;

        position_t next = board_step(tank->pos, dir);
        if (board_in_bounds(next))
            if (game.board[next.row][next.col] == CELL_EMPTY)
                if (game.bullets[next.row][next.col] == 0){
                    
                    position_t old = tank->pos;
                    game.board[old.row][old.col] = CELL_EMPTY;
                    game.board[next.row][next.col] = tank->name;
                    tank->pos = next;

                    ui_draw_cell(old);
                    ui_draw_cell(next);
                    ui_refresh();
                }
                
    }

    pthread_mutex_unlock(&game.board_lock);
}

static void *tank_thread(void *arg)
{
    tank_t *tank = arg;

    while(1){
        // doarme pana cand main pune o tasta in coada sau il trezesc manual
        sem_wait(tank->keys_available);

        if (!game_is_running())
            break;

        int key = tank_pop_key(tank);
        if (key < 0)
            continue;

        const controls_t *c = &tank->controls;
        if (key == c->fire)
            bullet_fire(tank);
        else if (key == c->up)
            tank_move(tank, DIR_UP);
        else if (key == c->down)
            tank_move(tank, DIR_DOWN);
        else if (key == c->left)
            tank_move(tank, DIR_LEFT);
        else if (key == c->right)
            tank_move(tank, DIR_RIGHT);
    }

    return NULL;
}

int tank_start(tank_t *tank)
{
    return pthread_create(&tank->thread, NULL, tank_thread, tank);
}
