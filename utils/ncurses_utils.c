/**
 * @file ncruses_utils.c
 * @brief une librairie qui simplifie l'utilisation de ncruses
**/

#include "ncurses_utils.h"

#include <ncurses.h>
#include <stdlib.h>



/**
 * @brief Initialisation de ncurses.
 * 
 */
void ncurses_initialiser() {
	initscr();				/* Demarre le mode ncurses */
	cbreak();				/* Pour les saisies clavier (desac. mise en buffer) */
	noecho();				/* Desactive l'affichage des caracteres saisis */
	keypad(stdscr, TRUE);	/* Active les touches specifiques */
	refresh();				/* Met a jour l'affichage */
	curs_set(FALSE);		/* Masque le curseur */
}

/**
 * @brief Fin de ncurses.
 * 
 */
void ncurses_stopper() {
	endwin();
}

/**
 * @brief Initialisation des couleurs.
 * 
 */
void ncurses_couleurs() {
	/* Verification du support de la couleur */
	if(has_colors() == FALSE) {
		ncurses_stopper();
		perror("Le terminal ne supporte pas les couleurs.\n");
		exit(EXIT_FAILURE);
	}

	use_default_colors();

	/* Activation des couleurs */
	start_color();

	/* Definition de la palette */
	init_pair(EAU_COLOR, COLOR_BLUE,  COLOR_BLUE);
	init_pair(POISSON_COLOR, COLOR_BLACK, COLOR_YELLOW);
	init_pair(JOUEUR_COLOR,  COLOR_RED,   COLOR_RED);
	init_pair(REQUIN_COLOR,  COLOR_WHITE, COLOR_YELLOW);
	init_pair(BOX_COLOR,     COLOR_WHITE, COLOR_BLACK);
	init_pair(RED_COLOR,     COLOR_RED, COLOR_BLACK);

}

/**
 * @brief Initialisation de la souris.
 * 
 */
void ncurses_souris() {
	if(!mousemask(ALL_MOUSE_EVENTS, NULL)) {
		ncurses_stopper();
		perror("Erreur lors de l'initialisation de la souris.\n");
		exit(EXIT_FAILURE);
	}

	if(has_mouse() != TRUE) {
		ncurses_stopper();
		perror("Aucune souris n'est détectée.\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief Recupere la position x et y de la souris.
 * 
 * @param x la position en x
 * @param y la position en y
 * @param bouton l'évenement associé au clic (ou NULL)
 * @return OK si reussite
 */
int souris_getpos(int *x, int *y, int *bouton) {
	MEVENT event;
	int resultat = getmouse(&event);

	if(resultat == OK) {
		*x = event.x;
		*y = event.y;
		if(bouton != NULL) *bouton = event.bstate;
	}
	return resultat;
}

/*PERSO*/

/**
 * @brief Créé une fentre box avec une sous fenêtre
 * 
 * @param win WINDOW** Un pointeur vers le pointeur de la fenêtre
 * @param h int La hauteur de la fenêtre
 * @param l int La largeur de la fenêtre
 * @param y int La position Y de la fenêtre
 * @param x int La position X de la fenêtre
 * 
 * @return WINDOW* un pointeur sur la fenetre supérieur 
 */
WINDOW* create_box(WINDOW** win, int h, int l, int y, int x){
	WINDOW *topWin = newwin(h, l, y, x);
	*win = newwin(h - 2, l - 2, y + 1, x + 1);
	box(topWin, 0, 0);
	wrefresh(topWin);
	return topWin;
}