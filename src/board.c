#include "board.h"
#include "game.h"

// '#' = zid, '1' / '2' = pozitiile de start ale tancurilor.
// Tabla nu are zid pe contur: marginea matricei este marginea tablei.
static const char *MAZE[BOARD_ROWS] = {
    "                                                            ",
    " 1          #                                #              ",
    "            #                                #              ",
    "    ####    #       ##########               #      ####    ",
    "                                                            ",
    "                    #        #       ######                 ",
    "   #######          #        #                 ########     ",
    "         #          #        #                        #     ",
    "         #                                            #     ",
    "         #       #######           #######                  ",
    "                                                  #         ",
    "   ####            #                    #         #         ",
    "      #            #     ########       #         ####      ",
    "      #            #                    #                   ",
    "                                                            ",
    "            ######          #                 #######       ",
    "    #                       #                               ",
    "    #           #           #           #                   ",
    "    #           #                       #                2  ",
    "                #                                           ",
};

void board_init(position_t *start_a, position_t *start_b)
{
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            char cell = MAZE[r][c];

            if (cell == '1') {
                start_a->row = r;
                start_a->col = c;
                cell = CELL_EMPTY;
            } else if (cell == '2') {
                start_b->row = r;
                start_b->col = c;
                cell = CELL_EMPTY;
            }
            game.board[r][c] = cell;
        }
    }
}

int board_in_bounds(position_t p)
{
    return p.row >= 0 && p.row < BOARD_ROWS && p.col >= 0 && p.col < BOARD_COLS;
}

position_t board_step(position_t p, direction_t dir)
{
    switch (dir) {
    case DIR_UP:    p.row--; break;
    case DIR_DOWN:  p.row++; break;
    case DIR_LEFT:  p.col--; break;
    case DIR_RIGHT: p.col++; break;
    }
    return p;
}
