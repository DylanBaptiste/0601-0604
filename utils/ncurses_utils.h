
#ifndef _NCURSES_UTILS_
#define _NCURSES_UTILS_
#include <ncurses.h>

#define POISSON_COLOR 0

void ncurses_initialiser();
void ncurses_stopper();
void ncurses_couleurs();
void ncurses_souris();
int souris_getpos(int *x, int *y, int *bouton);

WINDOW* create_box(WINDOW** win, int h, int l, int y, int x);

#endif
