#include <string.h>

#include "game.h"

game_t game;

void game_init(void)
{
    memset(game.bullets, 0, sizeof(game.bullets));
    game.isRunning = 1;
    game.winner = -1;
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
    int running = game.isRunning;
    pthread_mutex_unlock(&game.board_lock);
    return running;
}

void game_stop(void)
{
    pthread_mutex_lock(&game.board_lock);
    game.isRunning = 0;
    pthread_mutex_unlock(&game.board_lock);
}
