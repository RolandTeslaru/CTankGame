#ifndef BOARD_H
#define BOARD_H

#define BOARD_ROWS 20
#define BOARD_COLS 60

#define CELL_EMPTY ' '
#define CELL_WALL  '#'

typedef struct
{
    int row;
    int col;
} position_t;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

// Incarca labirintul in game.board si intoarce pozitiile de start ale tancurilor.
void board_init(position_t *start_a, position_t *start_b);

int board_in_bounds(position_t p);

// Pozitia vecina in directia data (poate iesi din tabla, se verifica cu board_in_bounds).
position_t board_step(position_t p, direction_t dir);

#endif
