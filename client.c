
#include "utils/ncurses_utils.h"
#include "utils/config.h"

#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 


typedef struct poisson_type{

    int fuite;
    int valeur;

}poisson_t;

void clean_ncurses(){
    ncurses_stopper();
    fprintf(stdout, "clean_ncurses\n");
    /*et delete fenetre*/
}



WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;
int largeur;
int hauteur;

int main(int argc, char** argv) {

    WINDOW *box_log, *box_jeu, *box_etat;
    int startMenu, poireaus;

    largeur = atoi(argv[1]);
    hauteur = atoi(argv[2]);

   /* if(argc != 6){
        fprintf(stdout, "mauvaise utilisation: ./client [<IP>] [<PORT>]\n");
		exit(EXIT_FAILURE);
    }else{
    }*/

    //envoye la reuqte UDP au serveur
    //




    /* ============ Mise en place graphique ============ */
    
    ncurses_initialiser();
    ncurses_couleurs();

    

	box_log  = create_box(&fenetre_log,  HAUTEUR1, LARGEUR1, POSY1, POSX1);
	box_jeu  = create_box(&fenetre_jeu,  hauteur + 2, largeur + 2, POSY2, POSX2);
	box_etat = create_box(&fenetre_etat, hauteur + 2, LARGEUR1 - (largeur + 2), POSY3, largeur + 2);
    
    mvwprintw(box_jeu,  0, 3, "Etang");
	mvwprintw(box_log,  0, 3, "Logs");
	mvwprintw(box_etat, 0, 3, "Etat");
	wrefresh(box_log );
	wrefresh(box_jeu );
	wrefresh(box_etat);
    
	scrollok(fenetre_log, TRUE);	
    startMenu = START_MENU;
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter:    CTRL+C");
    ++startMenu;
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    

    attron(COLOR_PAIR(POISSON_COLOR));
    mvwaddch( fenetre_jeu, 1, 1, 'P');
    attroff(COLOR_PAIR(3));

    wrefresh(fenetre_jeu);
    wrefresh(fenetre_etat);
    wrefresh(fenetre_log);


    getch();
    ncurses_stopper();


    exit(1);
}