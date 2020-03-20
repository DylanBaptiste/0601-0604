
#include "utils/ncurses_utils.h"
#include "utils/config.h"

#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>
#include <sys/time.h>

#define MAX_POISSON 1
#define POISSON 1
#define JOUEUR 2
#define VIDE 0

#define DEBUG 0

#define SPAWN_RATE_1 50
#define SPAWN_RATE_2 35
#define SPAWN_RATE_3 15

typedef struct poisson_type{
    int fuite;
    char valeur;
    int poireaus;
    pthread_mutex_t mutex;
    

}poisson_t;

typedef struct joueur_type{
    int number;
    int poireaus;
	pthread_mutex_t mutex;
    pthread_cond_t releve;
}joueur_t;

typedef struct case_type{
    int element;
    pthread_mutex_t mutex;
    union{
        poisson_t* poisson;
        joueur_t* joueur;
    };
}case_t;


joueur_t* joueur1;
   

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
    switch (p->valeur)
    {
    case '1': p->poireaus = 100; break;
    case '2': p->poireaus = 200; break;
    case '3': p->poireaus = 500; break;
    default:  p->poireaus = 100;break;
    }
    p->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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
    int catch_up, catch_down, catch_left, catch_right = 0;
    int i, r = 0;
	int attrape = 0;
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

    while(attrape == 0){
        
        up, down, left, right = 0;

        /*wprintw(fenetre_log, "\nFuite: %d", poisson->fuite);*/
        if(poisson->fuite > 0){
            poisson->fuite--;
        }
        
		
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
                                    if(DEBUG){
                                        wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                        mvwaddch(fenetre_jeu, y+1, x, 'L');
                                        wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                    }
                                    break;
                                }
                                case JOUEUR: {
                                    catch_down = poisson->fuite == 0; /*s'il est en fuite il ne se fait pas prendre*/
                                    if(catch_down == 0) unlock( y+1, x);
                                    down = 0;
                                    wprintw(fenetre_log, "\nJoueur en (%d, %d)", (y + 1), x);
                                    
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
                                if(DEBUG){ wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y-1, x, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));}
                            }else{
                                unlock((y - 1), x);
                            }  
                            break;
                        }    
                        case EBUSY:  /*wprintw(fenetre_log, "\n%d deja lock up (%d, %d)", number, (y - 1), x);*/ break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( x < largeur - 1){
                    switch(pthread_mutex_trylock(&map[largeur*y+(x + 1)].mutex)){
                        case 0:{
                            if( is_free(y, (x + 1)) ){
                                right = 1;
                                 if(DEBUG){wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y, x+1, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));}
                            }else{
                                unlock(y, (x + 1));
                            }  
                            break;
                        }    
                        case EBUSY:  /*wprintw(fenetre_log, "\n%d deja lock right (%d, %d)", number, y, (x + 1));*/ break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( x > 0){
                    switch(pthread_mutex_trylock(&map[largeur*y+(x - 1)].mutex)){
                        case 0:{
                            if( is_free(y, (x - 1)) ){
                                left = 1;
                                if(DEBUG){ wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                                mvwaddch(fenetre_jeu, y, x-1, 'L');
                                wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));}
                            }else{
                                unlock(y, (x - 1));
                            }  
                            break;
                        }    
                        case EBUSY: /* wprintw(fenetre_log, "\n%d deja lock left (%d, %d)", number, y, (x - 1)); */break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }

                if(catch_up || catch_down || catch_right || catch_left){
                    /*array = malloc(sizeof(char)*(catch_up+catch_down+catch_right+catch_left));
                    i=0;
                    if(catch_up) array[i++] = 'u';
                    if(catch_down) array[i++] = 'd';
                    if(catch_right) array[i++] = 'r';
                    if(catch_left) array[i++] = 'l';
                    r = random_range(0, i-1);
                    switch(array[r]){
                        case 'u': step(poisson, y, x, y-1, x); y--; break;
                        case 'd':{
                            
                        } break;
                        case 'r': step(poisson, y, x, y, x+1); x++; break;
                        case 'l': step(poisson, y, x, y, x-1); x--; break;
                        default: exit(EXIT_FAILURE);
                    }*/

                    wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                    mvwaddch(fenetre_jeu, y, x, 'P');
                    wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));

                    struct timespec timeToWait;
                    struct timeval now;
                    
                    gettimeofday(&now, NULL);

                    timeToWait.tv_sec = now.tv_sec+5;
                    timeToWait.tv_nsec = 0;

                    /*pthread_mutex_lock(&(poisson->mutex));*/
                    switch(pthread_cond_timedwait(&(map[largeur*(y+1)+x].joueur->releve), &(map[largeur*(y+1)+x].joueur->mutex), &timeToWait)){
                        case ETIMEDOUT:
							wprintw(fenetre_log, "\nLe temps est passé");
                            break;
						case EINVAL: perror("argument invalide pthread_cond_timedwait"); exit(EXIT_FAILURE); break;
						case EPERM: perror("mutex invalide pthread_cond_timedwait"); exit(EXIT_FAILURE); break;
						default: 
							wprintw(fenetre_log, "\nAttrapé !!!!");
                            joueur1->poireaus += poisson->poireaus;
                            mvwprintw(fenetre_etat, 0, 0, "Poireaus: %d", joueur1->poireaus);
							attrape = 1;
							break;
                    }
                    

					if(attrape){
						map[largeur*y+x].element = VIDE;
						wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
						mvwaddch(fenetre_jeu, old_y, old_x, ' ');
						wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
                        pthread_mutex_lock(&(map[largeur*(y+1)+x].joueur->mutex));
                        map[largeur*(y+1)+x].joueur->poireaus += poisson->valeur;
					}else{
                        poisson->fuite = 5;
                    }

                    unlock( y+1, x);

					catch_down = catch_left = catch_right = catch_up = 0;
					if(up) unlock(old_y-1, old_x);
					if(down) unlock(old_y+1, old_x);
					if(right) unlock(old_y, old_x+1);
					if(left)unlock(old_y, old_x-1);
					unlock(y, x);
					


                    

                }else{
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
                        
                        /*sleep(1);*/
                        
                        
                        if(up) unlock(old_y-1, old_x);
                        if(down) unlock(old_y+1, old_x);
                        if(right) unlock(old_y, old_x+1);
                        if(left)unlock(old_y, old_x-1);

                        free(array);
						unlock(old_y, old_x);
                        
                    }
					

                }
            break;
            }
            case EBUSY: {
                wprintw(fenetre_log, "\nJe suis lock", y, x);
                
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
    exit(EXIT_SUCCESS);
}


typedef struct coord_t{ int x, y;} coord_t;

void* routine_lock(void* arg){
    coord_t coord = *((coord_t*)arg);
    
    pthread_mutex_lock(&map[largeur*coord.y+coord.x].mutex);
    wprintw(fenetre_log, "\nlock: (%d %d)", coord.y, coord.x);

    if(DEBUG){wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
    mvwaddch(fenetre_jeu, coord.y, coord.x, 'L');
    wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));}
    

    switch(map[largeur*coord.y+coord.x].element){
        case POISSON:{
            /*pthread_mutex_lock(&map[largeur*coord.y+coord.x].poisson->mutex);*/
            map[largeur*coord.y+coord.x].poisson->fuite = 5;            
            /*pthread_mutex_unlock(&map[largeur*coord.y+coord.x].poisson->mutex);*/
        }
        default: break;
    }
    
    free(arg);
}
 

int main(int argc, char** argv) {

    WINDOW *box_log, *box_jeu, *box_etat, *box_magasin;
    int startMenu, poireaus, i, j ,k, sourisX, sourisY, bouton, click_y, click_x;
	int lignePose = 0;
	int ligneX, ligneY = 0;

    joueur1 = malloc(sizeof(joueur_t));
    *joueur1 = (joueur_t){1, 0, (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER, (pthread_cond_t)PTHREAD_COND_INITIALIZER};

    pthread_t lock_at_click[5*5];
     if(atexit(clean_ncurses) != 0) {
        perror("Probleme lors de l'enregistrement clean_ncurses");
       exit(EXIT_FAILURE);
    }

    


    largeur = atoi(argv[1]);
    hauteur = atoi(argv[2]);

    
    

    signal(SIGINT, handler); /*Liberation memoire gerer si SIGINT recus*/
    map = malloc(sizeof(case_t) * largeur * hauteur);

    
    k = 0;
    for(i = 0; i < largeur; ++i){
       for(j = 0; j < hauteur; ++j, ++k){
           map[k].element = VIDE;
           map[k].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
           /*pthread_mutex_init(&map[k].mutex, NULL);*/
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

    mvwprintw(fenetre_etat, 0, 0, "Poireaus: %d", joueur1->poireaus);
    
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
        
        threads_poissons[i] = (pthread_t*) malloc(sizeof(pthread_t));
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
        wrefresh(fenetre_etat);

        switch(getch()) {
            case KEY_MOUSE:
                if(souris_getpos(&sourisX, &sourisY, &bouton) == OK) {
                    if( (sourisX > 0 && sourisX <= largeur) && (sourisY > HAUTEUR_TOP && sourisY <= HAUTEUR_TOP + hauteur) ){
                        
                        click_y = sourisY - HAUTEUR_TOP - 1;
                        click_x = sourisX - 1;

						if(lignePose == 0){
							ligneX = click_x;
							ligneY = click_y;
                        	coord_t* tmp_coord;
							k=0;
							for(i = click_y-2; i < click_y-2 + 5; ++i){
								for(j = click_x-2; j < click_x-2 + 5; ++j, ++k){

									
									if( i >= 0 && i < hauteur && j < largeur  && j >= 0 ){
                                        
										tmp_coord = malloc(sizeof(coord_t));
										tmp_coord->x = j;
										tmp_coord->y = i;
										
										pthread_create(&lock_at_click[k], NULL, routine_lock, tmp_coord);
									}
								}
							}

							k = 0;
							for(i = click_y-2; i < click_y-2 + 5; ++i){
								for(j = click_x-2; j < click_x-2 + 5; ++j, ++k){
									
									if( i >= 0 && i < hauteur && j < largeur && j >= 0 ){
										pthread_join(lock_at_click[k], NULL);
									}
								}
							}


							switch (map[largeur*click_y+click_x].element)
							{
								case VIDE:{
									lignePose = 1;
									map[largeur*click_y+click_x].element = JOUEUR;
									map[largeur*click_y+click_x].joueur = joueur1;
									wattron(fenetre_jeu, COLOR_PAIR(JOUEUR_COLOR));
									mvwaddch(fenetre_jeu, click_y, click_x, 'J');
									wattroff(fenetre_jeu, COLOR_PAIR(JOUEUR_COLOR));
									break;
								}
								case POISSON: wprintw(fenetre_log, "\nImpossible de poser sa ligne ici");
								default: wprintw(fenetre_log, "\nImpossible de poser sa ligne ici"); break;
							}

							

							

							for(i = click_y-2; i < click_y-2 + 5; ++i){
								for(j = click_x-2; j < click_x-2 + 5; ++j){
								if( i >= 0 && i < hauteur && j < largeur && j >= 0 ){                                    
										unlock(i, j);
										if(DEBUG && !(i ==click_y && j == click_x)){
											wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
											mvwaddch(fenetre_jeu, i, j, ' ');
											wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
										}
									}
								}
							}
						}
						else{
							lignePose = 0;
							
							pthread_cond_broadcast(&(map[largeur*ligneY+ligneX].joueur->releve));


							map[largeur*ligneY+ligneX].element = VIDE;
							wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
							mvwaddch(fenetre_jeu, ligneY, ligneX, ' ');
							wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
						}
                        
                    }
                }
                break;
            default: break;
        }

       

        

    }


    exit(1);
}