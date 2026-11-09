#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "game.h"
#include "tank.h"
#include "ui.h"

#define QUIT_KEY 'q'
#define NUM_KEYS 5

static void usage(const char *prog)
{
    fprintf(stderr,
            "Folosire: %s LITERA1 TASTE1 LITERA2 TASTE2\n"
            "  TASTE = 5 caractere: sus jos stanga dreapta foc\n"
            "  exemplu: %s A wsadf B ikjlh\n"
            "  tasta '%c' este rezervata pentru iesire\n",
            prog, prog, QUIT_KEY);
}

// Verifica argumentele: litere distincte, cate 5 taste, toate cele 10 taste diferite.
static int parse_args(int argc, char *argv[], char names[2], const char *keys[2])
{
    if (argc != 5)
        return -1;

    int used[256] = { 0 }; // notez ce taste am folosit deja

    for (int i = 0; i < 2; i++) {
        const char *name = argv[1 + 2 * i];
        const char *k = argv[2 + 2 * i];

        if (strlen(name) != 1 || !isalpha((unsigned char)name[0])) {
            fprintf(stderr, "Numele tancului trebuie sa fie o singura litera: '%s'\n", name);
            return -1;
        }
        if (strlen(k) != NUM_KEYS) {
            fprintf(stderr, "Tancul %s trebuie sa aiba exact %d taste: '%s'\n", name, NUM_KEYS, k);
            return -1;
        }
        for (int j = 0; j < NUM_KEYS; j++) {
            unsigned char key = (unsigned char)tolower((unsigned char)k[j]);
            if (key == QUIT_KEY || !isgraph(key) || used[key]) {
                fprintf(stderr, "Tasta '%c' este invalida sau folosita de doua ori\n", k[j]);
                return -1;
            }
            used[key] = 1;
        }
        names[i] = name[0];
        keys[i] = k;
    }

    if (names[0] == names[1]) {
        fprintf(stderr, "Tancurile trebuie sa aiba litere diferite\n");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    char names[2];
    const char *keys[2];

    // Parsam argumentele si daca nu retuneaza 0 inseamna ca ceva a fost introdus gresit
    if (parse_args(argc, argv, names, keys) != 0) {
        usage(argv[0]);
        return 1;
    }

    game_init();

    position_t start[2];
    board_init(&start[0], &start[1]);

    // Tank start directions
    direction_t start_dir[2] = { DIR_RIGHT, DIR_LEFT };
    // initializam tankurile
    for (int i = 0; i < 2; i++) {
        if (tank_init(&game.tanks[i], i, names[i], keys[i], start[i], start_dir[i]) != 0) {
            if (i == 1)
                tank_destroy(&game.tanks[0]);
            game_destroy();
            return 1;
        }
        game.board[start[i].row][start[i].col] = names[i];
    }

    if (ui_init() != 0) {
        tank_destroy(&game.tanks[0]);
        tank_destroy(&game.tanks[1]);
        game_destroy();
        return 1;
    }

    // initial render
    pthread_mutex_lock(&game.board_lock);
    ui_draw_all();
    pthread_mutex_unlock(&game.board_lock);

    // apelem tnak start la ambele tankuri
    int started = 0;
    for (; started < 2; started++)
        if (tank_start(&game.tanks[started]) != 0)
            break;

    if (started < 2)
        game_stop();


    // Input While 
    while (game_is_running()) {
        int key = getch();     // revine dupa 100 ms si fara tasta (timeout din ui_init)
        if (key == ERR)
            continue;
        if (key < 256)
            key = tolower(key);

        if (key == QUIT_KEY) {
            game_stop();
            break;
        }
        for (int i = 0; i < 2; i++)
            if (tank_owns_key(&game.tanks[i], key))
                tank_push_key(&game.tanks[i], key);
    }

    // Cleanup la tankuri

    // inchdem thread urile la tankuri
    for (int i = 0; i < started; i++) {
        tank_wake(&game.tanks[i]); // threadurile pot fii blocate in sem wait
        pthread_join(game.tanks[i].thread, NULL);
    }

    pthread_mutex_lock(&game.board_lock);
    while (game.active_bullets > 0)
        pthread_cond_wait(&game.bullets_done, &game.board_lock);
    int winner = game.winner;
    pthread_mutex_unlock(&game.board_lock);

    // winner = -1      cnv a aparasat q pentru exit
    // winner >= 0      jocul s a terminat printr o lovitura finala
    if (winner >= 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Tancul %c a castigat! Apasa orice tasta.",
                 game.tanks[winner].name);
        ui_show_message(msg);
        flushinp(); // arunca toate tastele deja apasate si inca necittie din buffer
        timeout(-1); // getch n o sa mai astepte 100ms
        getch();
    }

    ui_end();

    if (winner >= 0)
        printf("Tancul %c a castigat!\n", game.tanks[winner].name);

    tank_destroy(&game.tanks[0]);
    tank_destroy(&game.tanks[1]);
    game_destroy();
    return 0;

}
