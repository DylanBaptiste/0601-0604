
#include "utils/ncurses_utils.h"
#include "utils/config.h"

#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>

#define MAX_POISSON 1
#define POISSON 1
#define VIDE 0

#define SPAWN_RATE_1 50
#define SPAWN_RATE_2 35
#define SPAWN_RATE_3 15

typedef struct poisson_type{
    int fuite;
    char valeur;

}poisson_t;

typedef struct case_type{
    int element;
    pthread_mutex_t mutex;
    union{
        poisson_t* poisson;
    };
}case_t;

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

int random_range(int min, int max){
    return (rand() % (max - min + 1)) + min;
}
void unlock(int y, int x){
    switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
        case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);
        case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE);
        case 0: default: break;
    }
}


poisson_t* generer_poisson(int seed){
    srand(time(0) + seed);
    int r = rand()%100;
    poisson_t* p = malloc(sizeof(poisson_t));
    p->fuite = 0;
    p->valeur = r < SPAWN_RATE_1 ? '1' : ( r < SPAWN_RATE_1 + SPAWN_RATE_2 ? '2' : '3' );
    return p;
}

int is_free(int y, int x){
    return map[largeur*y+x].element == VIDE;
}

void step(poisson_t* poisson, int y, int x, int next_y, int next_x){
    
    map[largeur*y+x].element = VIDE;
    map[largeur*next_y+next_x].element = POISSON;
    map[largeur*next_y+next_x].poisson = poisson;

    wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
    mvwaddch(fenetre_jeu, y, x, ' ');
    wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));

    wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
    mvwaddch(fenetre_jeu, next_y, next_x, poisson->fuite > 0 ? 'F' : poisson->valeur);
    wattroff(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
}

void* routine_poisson(void* arg){
    int number = *((int*)arg);
    int seed = *((int*)arg) * 10;
    int up, down, left, right = 0;
    int i, r = 0;
    char* array;
    poisson_t* poisson = generer_poisson(seed);
    int x, y, ok, direction, old_x, old_y = 0;
    number = poisson->valeur;
    srand(time(NULL));
    y = random_range(0, hauteur - 1);
    x = random_range(0, largeur - 1);
    ok = 0;
    while(!ok){
        switch (pthread_mutex_trylock(&map[largeur*y+x].mutex)){
            case 0:{
                    wprintw(fenetre_log, "\n%d lock (%d, %d)",number,  y, x);
                    if(is_free(y, x)){
                        ok = 1;
                        map[largeur*y+x].element = POISSON;
                        map[largeur*y+x].poisson = poisson;
                        wattron(fenetre_jeu, COLOR_PAIR(POISSON_COLOR));
                        mvwaddch(fenetre_jeu, y, x, poisson->valeur);
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

                        y = random_range(0, hauteur - 1);
                        x = random_range(0, largeur - 1);
                        sleep(1);
                    }
                }
                break;
            case EBUSY:{
                wprintw(fenetre_log, "\ndeja lock (%d, %d)", y, x);
                y = random_range(0, hauteur - 1);
                x = random_range(0, largeur - 1);
                /*sleep(1);*/
            }
            break;

            case EINVAL: perror("Le mutex n'a pas été initialisé"); exit(EXIT_FAILURE); break;
            
            default: break;
        }

        
        
    }

    while(1){
        
        up, down, left, right = 0;
        poisson->fuite -= poisson->fuite != 0;
        old_x = x;
        old_y = y;

        //try lock moi meme si echec -> quelque chose m'arrive je fait pas les if
        switch(pthread_mutex_trylock(&map[largeur*y+x].mutex)){
            case 0:{
                if( y < hauteur - 1){
                    switch(pthread_mutex_trylock(&map[largeur*(y + 1)+x].mutex)){
                        case 0:{
                            switch (map[largeur*(y + 1)+x].element)
                            {
                                case VIDE: {
                                    down = 1;
                                    /*wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                    mvwaddch(fenetre_jeu, y+1, x, 'L');
                                    wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));*/
                                    break;
                                }
                                case POISSON:
                                default: unlock((y + 1), x); break;
                            }
                            break;
                        }  
                        case EBUSY:  wprintw(fenetre_log, "\n%d deja lock down (%d, %d)", number, (y + 1), x); break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( y > 0){
                    switch(pthread_mutex_trylock(&map[largeur*(y - 1)+x].mutex)){
                        case 0:{
                            if( is_free((y - 1), x) ){
                                up = 1;
                                /*wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y-1, x, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));*/
                            }else{
                                unlock((y - 1), x);
                            }  
                            break;
                        }    
                        case EBUSY:  wprintw(fenetre_log, "\n%d deja lock up (%d, %d)", number, (y - 1), x); break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( x < largeur - 1){
                    switch(pthread_mutex_trylock(&map[largeur*y+(x + 1)].mutex)){
                        case 0:{
                            if( is_free(y, (x + 1)) ){
                                right = 1;
                                /*wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y, x+1, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));*/
                            }else{
                                unlock(y, (x + 1));
                            }  
                            break;
                        }    
                        case EBUSY:  wprintw(fenetre_log, "\n%d deja lock right (%d, %d)", number, y, (x + 1)); break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( x > 0){
                    switch(pthread_mutex_trylock(&map[largeur*y+(x - 1)].mutex)){
                        case 0:{
                            if( is_free(y, (x - 1)) ){
                                left = 1;
                                /*wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y, x-1, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));*/
                            }else{
                                unlock(y, (x - 1));
                            }  
                            break;
                        }    
                        case EBUSY:  wprintw(fenetre_log, "\n%d deja lock left (%d, %d)", number, y, (x - 1)); break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if(up || down || right || left){
            
                    i = 0;
                    array = malloc(sizeof(char)*(up+down+right+left));            
                    if(up) array[i++] = 'u';
                    if(down) array[i++] = 'd';
                    if(right) array[i++] = 'r';
                    if(left) array[i++] = 'l';

                    r = random_range(0, i-1);
                    switch(array[r]){
                        case 'u': step(poisson, y, x, y-1, x); y--; break;
                        case 'd': step(poisson, y, x, y+1, x); y++; break;
                        case 'r': step(poisson, y, x, y, x+1); x++; break;
                        case 'l': step(poisson, y, x, y, x-1); x--; break;
                        default: exit(EXIT_FAILURE);
                    }

                    
                    
                    if(up){
                        unlock(old_y-1, old_x);
                        
                        /*if((int)array[r] != 'u'){
                            wprintw(fenetre_log, "!");
                            wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                            mvwaddch(fenetre_jeu, old_y-1, old_x, ' ');
                            wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        }*/
                    }
                    if(down){
                        unlock(old_y+1, old_x);
                        
                        /*if(array[r] != 'd'){
                            wprintw(fenetre_log, "!");
                            wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                            mvwaddch(fenetre_jeu, old_y+1, old_x, ' ');
                            wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        }*/
                    }
                    if(right){
                        unlock(old_y, old_x+1);
                        
                        /*if(array[r] != 'r'){
                            wprintw(fenetre_log, "!");
                            wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                            mvwaddch(fenetre_jeu, old_y, old_x+1, ' ');
                            wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        }*/
                    }
                    if(left){
                        unlock(old_y, old_x-1);
                        
                        /*if(array[r] != 'l'){
                            wprintw(fenetre_log, "!");
                            wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                            mvwaddch(fenetre_jeu, old_y, old_x-1, ' ');
                            wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        }*/
                    }

                    free(array);
                    unlock(old_y, old_x);

                }
            break;
            }
            case EBUSY: {
                wprintw(fenetre_log, "\nJe suis lock", number, y, x);
                
                break;
            }
            case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
        }


        up = down = right = left = 0;
        sleep(1);
        
        

    }
}



void handler(int s){
    fprintf(stdout, "\nSignal recu: %d\n", s);
    quitter = s == SIGINT;
}




int main(int argc, char** argv) {

    WINDOW *box_log, *box_jeu, *box_etat, *box_magasin;
    int startMenu, poireaus, i, j ,k, sourisX, sourisY, bouton, click_y, click_x;

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
    ncurses_souris();

	
	
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

    timeout(30);
    while(!quitter){

        wrefresh(fenetre_log);
        wrefresh(fenetre_jeu);

        switch(getch()) {
            case KEY_MOUSE:
                if(souris_getpos(&sourisX, &sourisY, &bouton) == OK) {
                    if( (sourisX > 0 && sourisX <= largeur) && (sourisY > HAUTEUR_TOP && sourisY <= HAUTEUR_TOP + hauteur) ){
                        
                        click_y = sourisY - HAUTEUR_TOP - 1;
                        click_x = sourisX - 1;
                        wprintw(fenetre_log, "\n(%d, %d)", click_y, click_x);
                        
                        pthread_mutex_lock(&map[largeur*click_y+click_x].mutex);
                        pthread_mutex_lock(&map[largeur*(click_y)+(click_x + 1)].mutex);
                        pthread_mutex_lock(&map[largeur*(click_y + 1)+(click_x + 1)].mutex);
                        pthread_mutex_lock(&map[largeur*(click_y + 1)+(click_x)].mutex);
                        pthread_mutex_lock(&map[largeur*(click_y + 1)+(click_x + 1)].mutex);
                        switch(map[largeur*(sourisY - HAUTEUR_TOP - 1)+(sourisX -1)].element){
                            case POISSON: map[largeur*(sourisY - HAUTEUR_TOP - 1)+(sourisX -1)].poisson->fuite = 5;
                            default: break;
                        }
                        unlock(click_y, click_x);
                        unlock(click_y, click_x + 1);
                    }
                }
                break;
            default: break;
        }

        

    }


    exit(1);
}