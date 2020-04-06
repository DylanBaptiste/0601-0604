
#ifndef _NCURSES_UTILS_
#define _NCURSES_UTILS_
#include <ncurses.h>
#include <string.h>

#define VIDE 9
#define POISSON 1
#define PNEU 2
#define DYNAMITE 3
#define REQUIN 4
#define JOUEUR 5
#define BOX_COLOR  6
/*
#define EAU_COLOR 1
#define POISSON_COLOR 2
#define JOUEUR_COLOR  3
#define REQUIN_COLOR  4

#define RED_COLOR  6
*/

void ncurses_initialiser();
void ncurses_stopper();
void ncurses_couleurs();
void ncurses_souris();
int souris_getpos(int *x, int *y, int *bouton);

WINDOW* create_box(WINDOW** win, int h, int l, int y, int x);

#endif
