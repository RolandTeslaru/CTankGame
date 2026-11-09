#include <string.h>

#include "game.h"

game_t game;

void game_init(void)
{
    for (int r = 0; r < BOARD_ROWS; r ++)
        for (int c = 0; c < BOARD_COLS; c ++)
            game.bullets[r][c] = 0;

    game.isRunning = 1;
    game.winner = -1; // -1 nimeni, in rest indexul tankului
    game.active_bullets = 0;

    pthread_mutex_init(&game.board_lock, NULL);
    pthread_mutex_init(&game.ui_lock, NULL);
    pthread_cond_init(&game.bullets_done, NULL);
}

void game_destroy(void)
{
    pthread_cond_destroy(&game.bullets_done);
    pthread_mutex_destroy(&game.ui_lock);
    pthread_mutex_destroy(&game.board_lock);
}

int game_is_running(void)
{
    pthread_mutex_lock(&game.board_lock);
    int isRunning = game.isRunning;
    pthread_mutex_unlock(&game.board_lock);
    return isRunning;
}

void game_stop(void)
{
    pthread_mutex_lock(&game.board_lock);
    game.isRunning = 0;
    pthread_mutex_unlock(&game.board_lock);
}
