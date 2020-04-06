
#include "utils/ncurses_utils.h"
#include "config.h"
#include "structures/send_map.h"

#include <sys/socket.h>  /* Pour socket */
#include <arpa/inet.h>   /* Pour sockaddr_in, inet_pton */
#include <string.h>      /* Pour memset */

#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>



int quitter = 0;

void handler(int);
void clean_ncurses();


int main(int argc, char** argv) {

    send_map_t *map, *old_map;
    int largeur;
    int hauteur;
    WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat, *fenetre_magasin, *box_log, *box_jeu, *box_etat, *box_magasin;
    int startMenu, poireaus, i, j ,k, sourisX, sourisY, bouton, click_y, click_x;
	int ligneX, ligneY = 0;
    int sockUDP, sockTCP;
    socklen_t len;
    ssize_t map_length;
    struct sockaddr_in adresseServeur;
    uint16_t PortTcpServeur;
    int buff[2];

    /* Vérification des arguments */
    if(argc != 5) {
        fprintf(stderr, "Usage : %s adresseServeur portServeur largeur hauteur\n", argv[0]);
        fprintf(stderr, "Où :\n");
        fprintf(stderr, "  adresseServeur : adresse IPv4 du serveur\n");
        fprintf(stderr, "  portServeur    : numéro de port du serveur\n");
        fprintf(stderr, "  largeur        : la largeur de la partie\n");
        fprintf(stderr, "  hauteur        : la hauteur de la partie\n");
        exit(EXIT_FAILURE);
    }

    largeur = atoi(argv[1]);
    hauteur = atoi(argv[2]);
    signal(SIGINT, handler); 
    

    len = sizeof(adresseServeur);
    /* Création de la socket UDP*/
    if((sockUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Erreur lors de la création de la socket ");
        exit(EXIT_FAILURE);
    }
    /* Création de l'adresse du serveur */
    memset(&adresseServeur, 0, sizeof(struct sockaddr_in));
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(atoi(argv[2]));
    if(inet_pton(AF_INET, argv[1], &adresseServeur.sin_addr) != 1) {
        perror("Erreur lors de la conversion de l'adresse ");
        exit(EXIT_FAILURE);
    }
    /* Préparation du message */
    memset(buff, 0, 2*sizeof(int));
    buff[0] = atoi(argv[3]);
    buff[1] = atoi(argv[4]);

    printf("%d %d\n", buff[0], buff[1]);

    /* Envoi du message */
    if(sendto(sockUDP, &buff, 2*sizeof(int), 0, (struct sockaddr*)&adresseServeur, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur lors de l'envoi du message ");
        exit(EXIT_FAILURE);
    }
    printf("Client : message envoyé.\n");

    if(recvfrom(sockUDP, &PortTcpServeur, sizeof(uint16_t), 0, NULL, NULL) == -1) {
        perror("Erreur lors de la réception du message ");
        exit(EXIT_FAILURE);
    }
    printf("Le port TCP du serveur est: %d.\n", PortTcpServeur);

    if(close(sockUDP) == -1) {
        perror("Erreur lors de la fermeture de la socket ");
        exit(EXIT_FAILURE);
    }

    /*création de la socket TCP*/
    if((sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Erreur lors de la création de la socket ");
        exit(EXIT_FAILURE);
    }
    /* Remplissage de la structure */
    memset(&adresseServeur, 0, sizeof(struct sockaddr_in));
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(PortTcpServeur);
    if(inet_pton(AF_INET, argv[1], &adresseServeur.sin_addr.s_addr) != 1) {
        perror("Erreur lors de la conversion de l'adresse ");
        exit(EXIT_FAILURE);
    }

    /* Connexion au serveur */
    if(connect(sockTCP, (struct sockaddr*)&adresseServeur, len) == -1) {
        perror("Erreur lors de la connexion ");
        exit(EXIT_FAILURE);
    }

    /*lire largeur et hauteur*/
    if (read(sockTCP, buff, 2*sizeof(int)) == -1)
    {
        perror("Erreur lors de la lecture du message ");
        exit(EXIT_FAILURE);
    }

    largeur = buff[0];
    hauteur = buff[1];

    map_length = largeur*hauteur*sizeof(send_map_t);

    map = malloc(map_length);
    old_map = malloc(map_length);

    printf("L'etang est de %d par %d\n", largeur, hauteur); sleep(1);
    /*fprintf(stdout, "La partie commence dans 3..."); fflush(stdout); sleep(1); fprintf(stdout, " 2..."); fflush(stdout); sleep(1); fprintf(stdout, " 1..."); fflush(stdout); sleep(1);*/

    ncurses_initialiser();
    ncurses_couleurs();
    ncurses_souris();

	box_etat    = create_box(&fenetre_etat,    HAUTEUR_TOP, LARGEUR_ETAT, POSY_ETAT, POSX_ETAT);
    box_magasin = create_box(&fenetre_magasin, HAUTEUR_TOP, LARGEUR_MAGASIN, POSY_MAGASIN, POSX_MAGASIN);
    box_log     = create_box(&fenetre_log,     HAUTEUR_TOP, LARGEUR_LOG, POSY_LOG, POSX_LOG);
    box_jeu     = create_box(&fenetre_jeu,     hauteur + 2, largeur + 2, POSY_ETANG, POSX_ETANG);

    if(atexit(clean_ncurses) != 0) {
        perror("Probleme lors de l'enregistrement clean_ncurses");
       exit(EXIT_FAILURE);
    }
    
    mvwprintw(box_jeu,     0, (largeur + 2)/2 - strlen(NAME_ETANG)/2,	  "%s", NAME_ETANG);
	mvwprintw(box_log,     0, LARGEUR_LOG/2 - strlen(NAME_LOG)/2,		  "%s", NAME_LOG);
	mvwprintw(box_etat,    0, LARGEUR_ETAT/2 - strlen(NAME_ETAT)/2,		  "%s", NAME_ETAT);
    mvwprintw(box_magasin, 0, LARGEUR_MAGASIN/2 - strlen(NAME_MAGASIN)/2, "%s", NAME_MAGASIN);

    wbkgd(box_log, COLOR_PAIR(BOX_COLOR));
    wbkgd(box_jeu, COLOR_PAIR(BOX_COLOR));
    wbkgd(box_etat, COLOR_PAIR(BOX_COLOR));
	wbkgd(box_magasin, COLOR_PAIR(BOX_COLOR));
    
	wrefresh(box_log );
	wrefresh(box_jeu );
	wrefresh(box_etat);
	wrefresh(box_magasin);

    wbkgd(fenetre_jeu, COLOR_PAIR(VIDE));
    wbkgd(fenetre_log, COLOR_PAIR(BOX_COLOR));
    wbkgd(fenetre_etat, COLOR_PAIR(BOX_COLOR));
	wbkgd(fenetre_magasin, COLOR_PAIR(BOX_COLOR));
    
	scrollok(fenetre_log, TRUE);	
    startMenu = START_MENU;

    mvwprintw(fenetre_etat, 0, 0, "Poireaus: %d", 0);
    mvwprintw(fenetre_etat, 1, 0, "Mode furtif: %d", 0);
    
    wattron(fenetre_etat, COLOR_PAIR(POISSON));
    mvwaddch(fenetre_etat, startMenu+0, 0, '1');
    mvwaddch(fenetre_etat, startMenu+1, 0, '2');
    mvwaddch(fenetre_etat, startMenu+2, 0, '3');
    wattroff(fenetre_etat, COLOR_PAIR(POISSON));
    
    mvwprintw(fenetre_etat,   startMenu, 1, ": 100");
    mvwprintw(fenetre_etat, ++startMenu, 1, ": 200");
    mvwprintw(fenetre_etat, ++startMenu, 1, ": 500");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter: CTRL+C");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");

    mvwprintw(fenetre_magasin, 1, 1, "150: PNEU");
    mvwprintw(fenetre_magasin, 2, 1, "200: DYNAMITE");
    mvwprintw(fenetre_magasin, 3, 1, "300: REQUIN");
    mvwprintw(fenetre_magasin, 4, 1, "500: MODE FURTIF");

    wrefresh(fenetre_jeu);
    wrefresh(fenetre_etat);
    wrefresh(fenetre_log);
    wrefresh(fenetre_magasin);

    timeout(30);
    while(!quitter){

        /*============================== UPDATE MAP ==============================*/
        if (read(sockTCP, map, map_length) == -1){
            perror("Erreur lors de la lecture du message ");
            exit(EXIT_FAILURE);
        }   

        k = 0;
        for(i = 0; i < hauteur; ++i){
            for(j = 0; j < largeur; ++j, ++k){
                if( memcmp( &old_map[k], &map[k], sizeof(send_map_t)) != 0 ){ /*Pour ne pas reecrire des cases identiques à l'ecran == (old_map[k].element != map[k].element) || (old_map[k].c != map[k].c)*/
                    wattron(fenetre_jeu, COLOR_PAIR( map[k].element ));
                    mvwaddch(fenetre_jeu, i, j, map[k].c);
                    wattroff(fenetre_jeu, COLOR_PAIR( map[k].element ));
                }
            }
        }
        memcpy(old_map, map, map_length);
        wrefresh(fenetre_log);
        wrefresh(fenetre_jeu);
        wrefresh(fenetre_etat);
        /*===========================================================================*/
        

        switch(getch()) {
            case KEY_MOUSE:
                if(souris_getpos(&sourisX, &sourisY, &bouton) == OK) {
                    if( (sourisX > 0 && sourisX <= largeur) && (sourisY > HAUTEUR_TOP && sourisY <= HAUTEUR_TOP + hauteur) ){
                        
                        buff[0] = sourisY - HAUTEUR_TOP - 1;
                        buff[1] = sourisX - 1;

                        wprintw(fenetre_log, "\nclick %d %d", buff[0], buff[1]);
                        wrefresh(fenetre_log);

                        if (write(sockTCP, buff, 2*sizeof(int)) == -1)
                        {
                            perror("Erreur lors de la l'ecriture du clique ");
                            exit(EXIT_FAILURE);
                        }
                        
                    }
                }
                break;
            default: break;
        }
    }

    exit(EXIT_SUCCESS);
}


void clean_ncurses(){
    ncurses_stopper();
    fprintf(stdout, "clean_ncurses\n");
    /*et delete fenetre*/
}


void handler(int s){
    fprintf(stdout, "\nSignal recu: %d\n", s);
    quitter = s == SIGINT;
    exit(EXIT_SUCCESS);
}