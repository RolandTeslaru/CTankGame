#ifndef TANK_H
#define TANK_H

#include <pthread.h>
#include <semaphore.h>
#include "board.h"

#define TANK_LIVES 10
#define KEY_QUEUE_SIZE 16

typedef struct {
    int up;
    int down;
    int left;
    int right;
    int fire;
} controls_t;

typedef struct {
    char name;
    int index;              // 0 sau 1, pozitia in game.tanks
    position_t pos;
    direction_t dir;        // directia de deplasare = directia in care trage
    int lives;
    controls_t controls;
    pthread_t thread;

    // "Cutia postala" cu taste: main produce, thread-ul tancului consuma.
    int key_queue[KEY_QUEUE_SIZE];
    int key_head;
    int key_count;
    pthread_mutex_t key_lock;   // protejeaza coada
    sem_t *keys_available;      // cate taste asteapta in coada
} tank_t;

int tank_init(tank_t *tank, int index, char name, const char *keys,
              position_t start, direction_t dir);
void tank_destroy(tank_t *tank);

int tank_start(tank_t *tank);

// Apelat din main: pune tasta in coada tancului si il trezeste.
void tank_push_key(tank_t *tank, int key);

// Trezeste thread-ul fara tasta, ca sa observe ca jocul s-a terminat.
void tank_wake(tank_t *tank);

// Este tasta una dintre comenzile tancului?
int tank_owns_key(const tank_t *tank, int key);

#endif
