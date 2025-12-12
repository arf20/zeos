/*

    arfminesweeper: Cross-plataform multi-frontend game
    Copyright (C) 2023 arf20 (Ángel Ruiz Fernandez)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    ansi.c: terminal with ANSI-color-compatible escape enhancements

*/

#include <plibc.h>

#include <frontconf.h>
#include <game.h>

#include "ansi.h"

static int size = 0;
static const int *board = NULL;

#define MARGIN_L 0
#define MARGIN_T 2

/* cursor */
static int curx = 0, cury = 0;

static void
printBoard() {
    printf("\e[2;0H");

    printf("+");
    for (int x = 0; x < size; x++)
        printf("-");
    printf("+\n");

    for (int y = 0; y < size; y++) {
        printf("|");
        for (int x = 0; x < size; x++) {
            if (x == curx && y == cury) /* set negative at cursor */
                printf("\e[7m");

            if (CHECK_CLEAR(BOARDXY(x, y))) {
                /* If clear, count surrounding cells and print n of mines */
                int n = gameGetSurroundingMines(x, y);

                switch (n) {
                    case 1: printf("\e[94m"); break;
                    case 2: printf("\e[92m"); break;
                    case 3: printf("\e[91m"); break;
                    case 4: printf("\e[34m"); break;
                    case 5: printf("\e[31m"); break;
                    case 6: printf("\e[36m"); break;
                    case 7: printf("\e[90m"); break;
                    case 8: printf("\e[37m"); break;
                }

                n ? printf("%d", n) : printf(" ");

                printf("\e[0m");
            }
            else if (CHECK_FLAG(BOARDXY(x, y))) {
                printf("\e[31mF\e[0m");
            }
            else printf("#"); /* uncleared */

            if (x == curx && y == cury) /* restore positive at cursor */
                printf("\e[27m");
        }
        printf("|\n");
    }

    printf("+");
    for (int x = 0; x < size; x++)
        printf("-");
    printf("+\n");
}

int
ansi_start(const int *lboard, int lsize) {
    board = lboard;
    size = lsize;

    /* Console game loop */
    printBoard();
    printf("\e[%d;%dH", MARGIN_T + 1 + cury, MARGIN_L + 1 + curx);

    char input = 0;
    int run = 1;
    while (run) {
        input = getchar();

        if (isalpha(input)) {
            input = tolower(input);
            switch (input) {
                case 'a': curx--; break;
                case 'd': curx++; break;
                case 'w': cury--; break;
                case 's': cury++; break;
                case 'f': gameFlagCell(curx, cury); break;
                case 'c': gameClearCell(curx, cury); break;
            }
        }
        if (curx < 0) curx = size - 1;
        if (cury < 0) cury = size - 1;
        if (curx >= size) curx = 0;
        if (cury >= size) cury = 0;

        printBoard();

        printf("\e[%d;%dH", MARGIN_T + 1 + cury, MARGIN_L + 1 + curx);

        if (gameGetState() == STATE_LOST) {
            printf(TXT_LOST);
            run = 0;
        }

        if (gameGetState() == STATE_WON) {
            printf(TXT_WON);
            run = 0;
        }
    }

    return 0;
}

void
ansi_destroy() {

}

