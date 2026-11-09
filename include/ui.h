#ifndef UI_H
#define UI_H

#include "board.h"

// Intoarce -1 daca terminalul e prea mic.
int ui_init(void);
void ui_end(void);

// Functiile de mai jos citesc starea jocului: se apeleaza cu board_lock luat.
void ui_draw_all(void);
void ui_draw_cell(position_t p);
void ui_draw_lives(void);
void ui_refresh(void);

void ui_show_message(const char *msg);

#endif
