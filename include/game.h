#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include "board.h"
#include "tank.h"

typedef struct {
    char board[BOARD_ROWS][BOARD_COLS];     // ziduri si tancuri
    int bullets[BOARD_ROWS][BOARD_COLS];    // cate proiectile sunt pe fiecare pozitie
    tank_t tanks[2];
    int isRunning;
    int winner;                             // indexul castigatorului, -1 daca nu e
    int active_bullets;                     // thread-uri de proiectil inca in viata

    // Ordinea de blocare: intai board_lock, apoi ui_lock.
    pthread_mutex_t board_lock;     // tabla, pozitii, vieti, isRunning, winner, active_bullets
    pthread_mutex_t ui_lock;        // apelurile ncurses
    pthread_cond_t bullets_done;    // semnalat cand active_bullets ajunge la 0
} game_t;

extern game_t game;

void game_init(void);
void game_destroy(void);

// Citeste isRunning cu board_lock luat.
int game_is_running(void);
void game_stop(void);

#endif
