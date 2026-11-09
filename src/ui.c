#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "game.h"

// Tabla se deseneaza in interiorul unui chenar, sub randul cu vietile.
#define LIVES_ROW  0
#define FRAME_TOP  1
#define BOARD_TOP  (FRAME_TOP + 1)
#define BOARD_LEFT 1
#define FRAME_BOTTOM (BOARD_TOP + BOARD_ROWS)
#define HELP_ROW   (FRAME_BOTTOM + 1)
#define SCREEN_COLS (BOARD_COLS + 2)

enum {
    PAIR_WALL = 1,
    PAIR_TANK_A,
    PAIR_TANK_B,
    PAIR_BULLET
};

int ui_init(void)
{
    initscr();
    if (LINES < HELP_ROW + 1 || COLS < SCREEN_COLS) {
        endwin();
        fprintf(stderr, "Terminalul trebuie sa aiba cel putin %d x %d caractere.\n",
                SCREEN_COLS, HELP_ROW + 1);
        return -1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);   // getch() revine dupa 100 ms chiar daca nu s-a apasat nimic

    if (has_colors()) {
        start_color();
        init_pair(PAIR_WALL,   COLOR_WHITE,  COLOR_BLUE);
        init_pair(PAIR_TANK_A, COLOR_GREEN,  COLOR_BLACK);
        init_pair(PAIR_TANK_B, COLOR_RED,    COLOR_BLACK);
        init_pair(PAIR_BULLET, COLOR_YELLOW, COLOR_BLACK);
    }
    return 0;
}

void ui_end(void)
{
    endwin();
}

static int tank_color(char name)
{
    return name == game.tanks[0].name ? PAIR_TANK_A : PAIR_TANK_B;
}

// Apelata cu ui_lock luat.
static void draw_cell_locked(position_t p)
{
    char cell = game.board[p.row][p.col];
    chtype ch;

    if (cell == CELL_WALL)
        ch = '#' | COLOR_PAIR(PAIR_WALL);
    else if (cell != CELL_EMPTY)
        ch = (chtype)cell | COLOR_PAIR(tank_color(cell)) | A_BOLD;
    else if (game.bullets[p.row][p.col] > 0)
        ch = '.' | COLOR_PAIR(PAIR_BULLET) | A_BOLD;
    else
        ch = ' ';

    mvaddch(BOARD_TOP + p.row, BOARD_LEFT + p.col, ch);
}

static void draw_lives_locked(void)
{
    char right[32];
    const tank_t *a = &game.tanks[0];
    const tank_t *b = &game.tanks[1];

    move(LIVES_ROW, 0);
    clrtoeol();

    attron(COLOR_PAIR(PAIR_TANK_A) | A_BOLD);
    mvprintw(LIVES_ROW, 0, "%c: %d", a->name, a->lives);
    attroff(COLOR_PAIR(PAIR_TANK_A) | A_BOLD);

    snprintf(right, sizeof(right), "%c: %d", b->name, b->lives);
    attron(COLOR_PAIR(PAIR_TANK_B) | A_BOLD);
    mvprintw(LIVES_ROW, SCREEN_COLS - (int)strlen(right), "%s", right);
    attroff(COLOR_PAIR(PAIR_TANK_B) | A_BOLD);
}

void ui_draw_all(void)
{
    pthread_mutex_lock(&game.ui_lock);

    erase();
    draw_lives_locked();

    mvaddch(FRAME_TOP, 0, ACS_ULCORNER);
    mvhline(FRAME_TOP, 1, ACS_HLINE, BOARD_COLS);
    mvaddch(FRAME_TOP, SCREEN_COLS - 1, ACS_URCORNER);
    mvvline(BOARD_TOP, 0, ACS_VLINE, BOARD_ROWS);
    mvvline(BOARD_TOP, SCREEN_COLS - 1, ACS_VLINE, BOARD_ROWS);
    mvaddch(FRAME_BOTTOM, 0, ACS_LLCORNER);
    mvhline(FRAME_BOTTOM, 1, ACS_HLINE, BOARD_COLS);
    mvaddch(FRAME_BOTTOM, SCREEN_COLS - 1, ACS_LRCORNER);

    for (int r = 0; r < BOARD_ROWS; r++)
        for (int c = 0; c < BOARD_COLS; c++)
            draw_cell_locked((position_t){ r, c });

    mvprintw(HELP_ROW, 0, "q = iesire");
    refresh();

    pthread_mutex_unlock(&game.ui_lock);
}

void ui_draw_cell(position_t p)
{
    pthread_mutex_lock(&game.ui_lock);
    draw_cell_locked(p);
    pthread_mutex_unlock(&game.ui_lock);
}

void ui_draw_lives(void)
{
    pthread_mutex_lock(&game.ui_lock);
    draw_lives_locked();
    pthread_mutex_unlock(&game.ui_lock);
}

void ui_refresh(void)
{
    pthread_mutex_lock(&game.ui_lock);
    refresh();
    pthread_mutex_unlock(&game.ui_lock);
}

void ui_show_message(const char *msg)
{
    pthread_mutex_lock(&game.ui_lock);
    move(HELP_ROW, 0);
    clrtoeol();
    attron(A_BOLD | A_REVERSE);
    mvprintw(HELP_ROW, 0, " %s ", msg);
    attroff(A_BOLD | A_REVERSE);
    refresh();
    pthread_mutex_unlock(&game.ui_lock);
}
