
#include "utils/ncurses_utils.h"
#include "utils/config.h"

#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>

#define MAX_POISSON 10
#define POISSON 1
#define VIDE 0


typedef struct poisson_type{

    int fuite;
    int valeur;

}poisson_t;

typedef struct case_type{
    int element;
    pthread_mutex_t mutex;
} case_t;

void clean_ncurses(){
    ncurses_stopper();
    fprintf(stdout, "clean_ncurses\n");
    /*et delete fenetre*/
}


int quitter = 0;
case_t* map;
pthread_mutex_t affichage = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threads_poissons[MAX_POISSON];

WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat, *fenetre_magasin;
int largeur;
int hauteur;

int randomRange(int min, int max){
    return (rand() % (max - min + 1)) + min;
}

int is_free(int y, int x){
    return map[largeur*y+x].element == VIDE;
}

void* routine_poisson(void* arg){
    int x, y, ok, direction, old_x, old_y = 0;
    int number = *((int*)arg);
    number += 65;
    srand(time(NULL));
    y = randomRange(0, hauteur - 1);
    x = randomRange(0, largeur - 1);
    ok = 0;
    while(!ok){
        switch (pthread_mutex_trylock(&map[largeur*y+x].mutex)){
            case 0:{
                    wprintw(fenetre_log, "\n%d lock (%d, %d)",number,  y, x);
                    if(is_free(y, x)){
                        ok = 1;
                        map[largeur*y+x].element = POISSON;
                        wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                        mvwaddch(fenetre_jeu, y, x, number);
                        wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                        
                        
                        switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                            case 0:      wprintw(fenetre_log, "\n%d unlock (%d, %d)",number,  y, x); break;
                            case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                            case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                            default: break;
                        }

                    }else{
                        wprintw(fenetre_log, "\n%d (%d, %d) non libre",number, y, x);

                        
                        switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                            case 0:      wprintw(fenetre_log, "\n%d unlock (%d, %d)",number, y, x); break;
                            case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                            case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                            default: break;
                        }

                        y = randomRange(0, hauteur - 1);
                        x = randomRange(0, largeur - 1);
                        sleep(1);
                    }
                }
                break;
            case EBUSY:{
                wprintw(fenetre_log, "\ndeja lock (%d, %d)", y, x);
                y = randomRange(0, hauteur - 1);
                x = randomRange(0, largeur - 1);
                sleep(1);
            }
            break;

            case EINVAL: perror("Le mutex n'a pas été initialisé"); exit(EXIT_FAILURE); break;
            
            default: wprintw(fenetre_log, "\n??"); break;
        }

        
        
    }
    
    
    old_x = x;
    old_y = y;

    while(1){

        /*wprintw(fenetre_log, "\n(%d, %d)", y, x);*/
        
        switch (randomRange(0, 0))
        {
            case 0:
                if( y < hauteur - 1){
                    
                    switch(pthread_mutex_trylock(&map[largeur*(y+1)+x].mutex)){
                        case 0:{
                            wprintw(fenetre_log, "\n%d lock (%d, %d)", number, y + 1, x);
                            if( is_free(y+1, x) ){
                                map[largeur*y+x].element = VIDE;
                                wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                                mvwaddch(fenetre_jeu, y, x, ' ');
                                wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                                y++;
                                map[largeur*y+x].element = POISSON;
                                wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                                mvwaddch(fenetre_jeu, y, x, number);
                                wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                                switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                                    case 0:      wprintw(fenetre_log, "\n%d unlock (%d, %d)",number, y, x); break;
                                    case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                                    case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                                    default: break;
                                }
                            }else{
                                switch(pthread_mutex_unlock(&map[largeur*(y+1)+x].mutex)){
                                    case 0:      wprintw(fenetre_log, "\n%d unlock (%d, %d)",number, y+1, x); break;
                                    case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                                    case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                                    default: break;
                                }
                            }  
                            break;
                        }    
                        case EBUSY:  wprintw(fenetre_log, "\n%d deja lock (%d, %d)", number, y+1, x); break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                break;
                case 1:{
                    if( y > 0){
                    
                        switch(pthread_mutex_trylock(&map[largeur*(y-1)+x].mutex)){
                            case 0:{
                                wprintw(fenetre_log, "\n%d lock (%d, %d)", number, y - 1, x); 
                                
                                map[largeur*y+x].element = VIDE;
                                wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                                mvwaddch(fenetre_jeu, y, x, ' ');
                                wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));

                                switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                                    case 0:      wprintw(fenetre_log, "\n%d unlock (%d, %d)",number, y, x); break;
                                    case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                                    case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                                    default: break;
                                }
                                y--;
                                map[largeur*y+x].element = POISSON;
                                wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                                mvwaddch(fenetre_jeu, y, x, number);
                                wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                                  
                                break;
                            }    
                            case EBUSY:  wprintw(fenetre_log, "\n%d deja lock (%d, %d)", number, y-1, x); break;
                            case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                            default: break;
                        }
                    }
                }
                break;
            /*case 1:
                if( y > 0 && is_free(y-1, x) ){
                    map[largeur*y+x].element = VIDE;
                    wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    mvwaddch(fenetre_jeu, y, x, ' ');
                    wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    y--;
                    map[largeur*y+x].element = POISSON;
                    wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                    mvwaddch(fenetre_jeu, y, x, 'P');
                    wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                }
                break;*/
            case 2:
                if( x < largeur - 1 && is_free(y, x+1) ){
                    map[largeur*y+x].element = VIDE;
                    wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    mvwaddch(fenetre_jeu, y, x, ' ');
                    wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    x++;
                    map[largeur*y+x].element = POISSON;
                    wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                    mvwaddch(fenetre_jeu, y, x, 'P');
                    wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                }
                break;
            case 3:
                if( x > 0 && is_free(y, x-1) ){
                    map[largeur*y+x].element = VIDE;
                    wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    mvwaddch(fenetre_jeu, y, x, ' ');
                    wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    x--;
                    map[largeur*y+x].element = POISSON;
                    wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                    mvwaddch(fenetre_jeu, y, x, 'P');
                    wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                }
                break;

            default: break;
        }

        /*wprintw(fenetre_log, " => (%d, %d)", y, x);*/
        
        sleep(1);

    }
}

void handler(int s){
    fprintf(stdout, "\nSignal recu: %d\n", s);
    quitter = s == SIGINT;
}




int main(int argc, char** argv) {

    WINDOW *box_log, *box_jeu, *box_etat, *box_magasin;
    int startMenu, poireaus, i, j ,k;

    largeur = atoi(argv[1]);
    hauteur = atoi(argv[2]);

    signal(SIGINT, handler); /*Liberation memoire gerer si SIGINT recus*/
    map = malloc(sizeof(case_t) * largeur * hauteur);

    for(i, k = 0; i < largeur; ++i){
       for(j = 0; j < hauteur; ++j, ++k){
           map[k].element = VIDE;
           pthread_mutex_init(&map[k].mutex, NULL);
        }
    }

   /* if(argc != 6){
        fprintf(stdout, "mauvaise utilisation: ./client [<IP>] [<PORT>]\n");
		exit(EXIT_FAILURE);
    }else{
    }*/

    //envoye la reuqte UDP au serveur
    //




    /* ============ Mise en place graphique ============ */
    
    ncurses_initialiser();
    if(atexit(clean_ncurses) != 0) {
        perror("Probleme lors de l'enregistrement clean_ncurses");
       exit(EXIT_FAILURE);
    }
    
    ncurses_couleurs();

	
	
	box_etat    = create_box(&fenetre_etat,    HAUTEUR_TOP, LARGEUR_ETAT, POSY_ETAT, POSX_ETAT);
    box_magasin = create_box(&fenetre_magasin, HAUTEUR_TOP, LARGEUR_MAGASIN, POSY_MAGASIN, POSX_MAGASIN);
    box_log     = create_box(&fenetre_log,     HAUTEUR_TOP, LARGEUR_LOG, POSY_LOG, POSX_LOG);
    box_jeu     = create_box(&fenetre_jeu,     hauteur + 2, largeur + 2, POSY_ETANG, POSX_ETANG);
    
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

    wbkgd(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
    wbkgd(fenetre_log, COLOR_PAIR(BOX_COLOR));
    wbkgd(fenetre_etat, COLOR_PAIR(BOX_COLOR));
	wbkgd(fenetre_magasin, COLOR_PAIR(BOX_COLOR));
    
	scrollok(fenetre_log, TRUE);	
    startMenu = START_MENU;

    mvwprintw(fenetre_etat, 0, 0, "Poireaus: %d", poireaus);
    
    wattron(fenetre_etat, COLOR_PAIR(POISSON_COLOR));
    mvwaddch(fenetre_etat, startMenu+0, 0, '1');
    mvwaddch(fenetre_etat, startMenu+1, 0, '2');
    mvwaddch(fenetre_etat, startMenu+2, 0, '3');
    wattroff(fenetre_etat, COLOR_PAIR(POISSON_COLOR));
    
    mvwprintw(fenetre_etat,   startMenu, 1, ": 100");
    mvwprintw(fenetre_etat, ++startMenu, 1, ": 200");
    mvwprintw(fenetre_etat, ++startMenu, 1, ": 500");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter: CTRL+C");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    

    wrefresh(fenetre_jeu);
    wrefresh(fenetre_etat);
    wrefresh(fenetre_log);

    
    for(i = 0; i < MAX_POISSON; i++){
        
        threads_poissons[i] = (pthread_t *) malloc(sizeof(pthread_t));
    }

    int* tmp;
    for(i = 0; i < MAX_POISSON; i++){
        tmp = malloc(sizeof(int*));
        *tmp = i;
        pthread_create(threads_poissons[i], NULL, routine_poisson, tmp);
        
    }

    while(!quitter){

       
      
        /*for (i = 0, k=0; i < largeur; i++)
        {
            for (j = 0; j < hauteur; j++, k++)
            {
                    if(!pthread_mutex_trylock(&map[k].mutex))
                    {
                        pthread_mutex_unlock(&map[k].mutex);
                    
                        wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                        mvwaddch(fenetre_jeu, j, i, 'L');
                        wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                    }else{
                        pthread_mutex_unlock(&map[k].mutex);
                        wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        mvwaddch(fenetre_jeu, j, i, ' ');
                        wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                    }
                    
            }
        }*/

        wrefresh(fenetre_log);
        wrefresh(fenetre_jeu);
        

        sleep(1);
    }


    exit(1);
}