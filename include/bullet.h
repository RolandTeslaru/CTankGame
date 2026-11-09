#ifndef BULLET_H
#define BULLET_H

#include "board.h"
#include "tank.h"

#define BULLET_DELAY_US 40000   // cat sta proiectilul pe o pozitie

typedef struct {
    position_t pos;
    direction_t dir;
    int owner;          // indexul tancului care a tras
    int on_board;       // 0 cat timp e inca "in teava" (pe pozitia tancului)
} bullet_t;

// Lanseaza un thread nou pentru un proiectil tras de tancul dat.
void bullet_fire(const tank_t *shooter);

#endif
