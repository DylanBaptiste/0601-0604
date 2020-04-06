#include "serveur_func.h"

#define CLIENTS_PER_GAME 2

/*Unlock la case y,x*/
void unlock(case_t* map, int largeur, int y, int x){
    
    switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
        case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);
        case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE);
        case 0: default: break;
    }
}



int random_range(int min, int max){ return (rand() % (max - min + 1)) + min; }

/*fait passer un poisson d'une case à l'autre*/
void step(send_map_t* send_map, case_t* map, int largeur, int y, int x, int next_y, int next_x){

    map[largeur*next_y+next_x].data.poisson = map[largeur*y+x].data.poisson;

    send_map[largeur*next_y+next_x].element = map[largeur*next_y+next_x].element = POISSON;
    send_map[largeur*next_y+next_x].c = map[largeur*next_y+next_x].data.poisson->fuite > 0 ? 'F' : map[largeur*next_y+next_x].data.poisson->valeur;
    
    send_map[largeur*y+x].element = map[largeur*y+x].element = VIDE;
    send_map[largeur*y+x].c = ' ';
}


void* routine_lock(void* arg){
    
    coord_t coord = *((coord_t*)arg);
    
    pthread_mutex_lock(&coord.map[coord.largeur*coord.y+coord.x].mutex);    

    switch(coord.map[coord.largeur*coord.y+coord.x].element){
        case POISSON:{
            coord.map[coord.largeur*coord.y+coord.x].data.poisson->fuite = 5;            
        }
        default: break;
    }
    
    free(arg);
    return 0;
}

int accept_all(int sockTCP, int Clients[CLIENTS_PER_GAME]){
    short i;
    for(i=0; i < CLIENTS_PER_GAME; ++i){
        if((Clients[i] = accept(sockTCP, NULL, NULL)) == -1) {
            /*perror("Connection du client impossible ");*/
            return -1;
        }
    }
    return 0;
}

int send_all(int Clients[CLIENTS_PER_GAME], const void* buff, size_t len){
    short i;
    for(i=0; i < CLIENTS_PER_GAME; ++i){
        if(write(Clients[i], buff, len) == -1){
            perror("Erreur lors de l'ecriture du message ");
            return -1;
        }
    }
    return 0;
}

typedef struct attr_r_send_t
{
    int* Clients;
    send_map_t* send_map;
    int largeur, hauteur;
}attr_r_send_t;


void* routine_send(void* arg){

    attr_r_send_t attr = *( (attr_r_send_t*)arg);
    int* Clients = attr.Clients;
    int largeur = attr.largeur;
    int hauteur = attr.hauteur;
    send_map_t* send_map = attr.send_map;
    free(arg);

    while(1){
        sleep(1);

        if( send_all(Clients, send_map, largeur*hauteur*sizeof(send_map_t)) == -1 ){
            perror("Erreur lors de l'envoie d'un message à un client");
        }
    }

}

void* routine_partie(void* arg){
    attr_r_partie attr = *( (attr_r_partie*)arg );
    
    int sockTCP = attr.sockTCP;
    int largeur = attr.largeur;
    int hauteur = attr.hauteur;
    int sockUDP = attr.sockUDP;
    uint16_t portTCP = attr.portTCP;
    struct sockaddr_in Client1 = attr.Client1;
    struct sockaddr_in Client2 = attr.Client2;
    int nb = attr.nb;
    int i, j, k;
    case_t* map;
    send_map_t* send_map;
    pthread_t *threads_poissons[MAX_POISSON];
    pthread_t* thread_send;

    attr_r_poisson* tmp;


    int Clients[2];
    int taillePartie[2];
    taillePartie[0] = largeur;
    taillePartie[1] = hauteur;  

    map = malloc(sizeof(case_t) * largeur * hauteur);
    send_map = malloc(sizeof(send_map_t) * largeur * hauteur);

    /*free(arg); il est alloué une fois par le main et c'est lui qui fera le free*/
    printf("UnLock %p\n", (void*)&(attr.mutex));
    pthread_mutex_unlock(attr.mutex);

    if(listen(sockTCP, 2) == -1) {
        perror("Erreur lors de la mise en mode passif ");
        exit(EXIT_FAILURE);
    }

    if(sendto(sockUDP, &portTCP, sizeof(uint16_t), 0, (struct sockaddr*)&Client1, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur lors de l'envoi du message ");
        exit(EXIT_FAILURE);
    }
    printf("\t%d: port TCP envoyé au Client 1\n", nb);
    
    if(sendto(sockUDP, &portTCP, sizeof(uint16_t), 0, (struct sockaddr*)&Client2, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur lors de l'envoi du message ");
        exit(EXIT_FAILURE);
    }
    printf("\t%d: Port TCP envoyé au Client 2\n", nb);

    printf("\t%d: Attente des connexion TCP des %d clients...\n", nb, CLIENTS_PER_GAME);
    accept_all(sockTCP, Clients);
    printf("\t%d: Les %d clients sont connectés\n", nb, CLIENTS_PER_GAME);

    if (send_all(Clients, taillePartie, 2*sizeof(int)) == -1){
        perror("Erreur lors de l'envoie d'un message à un client");
    } 
    printf("\t%d: Les deux client ont recu la taille de la map\n", nb);

    k = 0;
    for(i = 0; i < largeur; ++i){
       for(j = 0; j < hauteur; ++j, ++k){
           send_map[k].element = VIDE;
           send_map[k].c =  ' ';
           map[k].element = VIDE;
           map[k].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        }
    }

    /*ont attend le decompte des clients*/
    /*sleep(4);*/

    for(i = 0; i < MAX_POISSON; i++){
        threads_poissons[i] = (pthread_t*) malloc(sizeof(pthread_t));
        tmp = malloc(sizeof(attr_r_poisson));
        tmp->map = map;
        tmp->hauteur = hauteur;
        tmp->largeur = largeur;
        tmp->number = i;
        tmp->game = nb;
        tmp->send_map = send_map;
        pthread_create(threads_poissons[i], NULL, routine_poisson, tmp);
    }

    thread_send = (pthread_t*)malloc(sizeof(pthread_t));
    attr_r_send_t* attr_send = malloc(sizeof(attr_r_send_t));
    attr_send->hauteur = hauteur;
    attr_send->largeur = largeur;
    attr_send->Clients = Clients;
    attr_send->send_map = send_map;
    pthread_create(thread_send, NULL, routine_send, attr_send);
    

    while(1) {  
        
        
        

        /*click_y = sourisY - HAUTEUR_TOP - 1;
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
        
    }*/

        

    }
    
    games[nb].start = 1;
    /* Fermeture de la socket TCP*/
    if(close(sockTCP) == -1) {
        perror("Erreur lors de la fermeture de la socket ");
        return (void*)-1;
    }
    
}





/*routine du thread du poisson*/
void* routine_poisson(void* arg){
    int hauteur, largeur, number, game;
    int up, down, left, right, catch_up, catch_down, catch_left, catch_right, i, r, attrape;
    int x, y, ok, old_x, old_y, tmp_y, tmp_x = 0;
    char* array;

    case_t* map;
    send_map_t* send_map;
    attr_r_poisson attr = *((attr_r_poisson*)arg);
    poisson_t* poisson;
    struct timespec timeToWait;
    struct timeval now;
    free(arg);
    
    hauteur = attr.hauteur;
    largeur = attr.largeur;
    number = attr.number;
    send_map = attr.send_map;
    game = attr.game;
    map = attr.map;
    
    poisson = generer_poisson(number);
    
    srand(time(NULL));
    y = random_range(0, hauteur - 1);
    x = random_range(0, largeur - 1);

    up = down = left = right = catch_up = catch_down = catch_left = catch_right = i = r = attrape = 0;
    
    ok = 0;
    while(!ok){
        switch (pthread_mutex_trylock(&map[largeur*y+x].mutex)){
            case 0:{
                    if( map[largeur*y+x].element == VIDE ){
                        ok = 1;
                        map[largeur*y+x].element = POISSON;
                        map[largeur*y+x].data.poisson = poisson;
                                              
                        
                        switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                            case 0:      break;
                            case EINVAL: perror("Le mutex n'est pas init"); exit(EXIT_FAILURE);  break;
                            case EPERM:  perror("Le thread appelant ne possède pas le mutex et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                            default: break;
                        }

                    }else{
                        switch(pthread_mutex_unlock(&map[largeur*y+x].mutex)){
                            case 0:      break;
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
                y = random_range(0, hauteur - 1);
                x = random_range(0, largeur - 1);
            }
            break;
            case EINVAL: perror("Le mutex n'a pas été initialisé"); exit(EXIT_FAILURE); break;
            default: break;
        }
    }

    while(attrape == 0){

        printf("\t%d: %d: \t(%d,%d)\n", game, number, y, x);
        
        up = down = left = right = 0;

        if(poisson->fuite > 0){
            poisson->fuite--;
        }
		
        old_x = x;
        old_y = y;

        switch(pthread_mutex_trylock(&map[largeur*y+x].mutex)){
            case 0:{
                if( y < hauteur - 1){
                    switch(pthread_mutex_trylock(&map[largeur*(y + 1)+x].mutex)){
                        case 0:{
                            switch (map[largeur*(y + 1)+x].element)
                            {
                                case VIDE: {
                                    down = 1;
                                    break;
                                }
                                case JOUEUR: {
                                    catch_down = poisson->fuite == 0; /*s'il est en fuite il ne se fait pas prendre*/
                                    if(catch_down == 0) unlock(map, largeur, y+1, x);
                                    down = 0;                                    
                                    break;
                                }
                                case POISSON: unlock(map, largeur, (y + 1), x); break;
                                default: unlock(map, largeur, (y + 1), x); break;
                            }
                            break;
                        }  
                        case EBUSY:  /*wprintw(fenetre_log, "\n%d deja lock down (%d, %d)", number, (y + 1), x);*/ break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }
                if( y > 0){
                    switch(pthread_mutex_trylock(&map[largeur*(y - 1)+x].mutex)){
                        case 0:{
                            switch (map[largeur*(y - 1)+x].element)
                            {
                                case VIDE: {
                                    up = 1;
                                    break;
                                }
                                case JOUEUR: {
                                    catch_up = poisson->fuite == 0; /*s'il est en fuite il ne se fait pas prendre*/
                                    if(catch_up == 0) unlock(map, largeur, y-1, x);
                                    up = 0;
                                    
                                    break;
                                }
                                case POISSON: unlock(map, largeur,(y - 1), x); break;
                                default: unlock(map, largeur,(y - 1), x); break;
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
                            switch (map[largeur*y+(x+1)].element)
                            {
                                case VIDE: {
                                    right = 1;
                                    break;
                                }
                                case JOUEUR: {
                                    catch_right = poisson->fuite == 0; /*s'il est en fuite il ne se fait pas prendre*/
                                    if(catch_right == 0) unlock(map, largeur, y, x+1);
                                    right = 0;
                                    
                                    break;
                                }
                                case POISSON: unlock(map, largeur,y, x+1); break;
                                default: unlock(map, largeur,y, x+1); break;
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
                            switch (map[largeur*y+(x-1)].element)
                            {
                                case VIDE: {
                                    left = 1;
                                    break;
                                }
                                case JOUEUR: {
                                    catch_left = poisson->fuite == 0; /*s'il est en fuite il ne se fait pas prendre*/
                                    if(catch_left == 0) unlock(map, largeur, y, x-1);
                                    left = 0;                                    
                                    break;
                                }
                                case POISSON: unlock(map, largeur,y, x-1); break;
                                default: unlock(map, largeur,y, x-1); break;
                            }
                            break;
                        }    
                        case EBUSY: /* wprintw(fenetre_log, "\n%d deja lock left (%d, %d)", number, y, (x - 1)); */break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
                        default: break;
                    }
                }

                if(catch_up || catch_down || catch_right || catch_left){
                    
                    array = malloc(sizeof(char)*(catch_up+catch_down+catch_right+catch_left));
                    gettimeofday(&now, NULL);
                    timeToWait.tv_sec = now.tv_sec+5;
                    timeToWait.tv_nsec = 0;
                    
                    i=0;
                    if(catch_up) array[i++] = 'u';
                    if(catch_down) array[i++] = 'd';
                    if(catch_right) array[i++] = 'r';
                    if(catch_left) array[i++] = 'l';

                    r = random_range(0, i-1);
                    switch(array[r]){
                        case 'u': tmp_y = y-1; tmp_x = x; break;
                        case 'd': tmp_y = y+1; tmp_x = x; break;
                        case 'r': tmp_y = y; tmp_x = x+1; break;
                        case 'l': tmp_y = y; tmp_x = x-1; break;
                        
                        default: exit(EXIT_FAILURE);
                    }

                    switch(pthread_mutex_trylock(&(map[largeur*tmp_y+tmp_x].data.joueur->mutex))){
                        case 0: 
                            /*wattron(fenetre_jeu, COLOR_PAIR(RED_COLOR));
                            mvwaddch(fenetre_jeu, y, x, 'P');
                            wattroff(fenetre_jeu, COLOR_PAIR(RED_COLOR));*/

                            switch(pthread_cond_timedwait(&(map[largeur*tmp_y+tmp_x].data.joueur->releve), &(map[largeur*tmp_y+tmp_x].data.joueur->mutex), &timeToWait)){
                                case 0: attrape = 1; poisson->fuite = 5; break;
                                case ETIMEDOUT:  attrape = 0; break;
                                case EINVAL: perror("argument invalide pthread_cond_timedwait"); exit(EXIT_FAILURE); break;
                                case EPERM: perror("mutex invalide pthread_cond_timedwait"); exit(EXIT_FAILURE); break;
                                default: break;
                            }
                            pthread_mutex_unlock(&(map[largeur*tmp_y+tmp_x].data.joueur->mutex));
                        break;
                        case EBUSY: break;
                        case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;

                    }
                    
					if(attrape){
                        /*map[largeur*tmp_y+tmp_x].joueur joueur1->poireaus += poisson->poireaus;*/
						map[largeur*y+x].element = VIDE;
						/*wattron(fenetre_jeu, COLOR_PAIR(EAU_COLOR));
						mvwaddch(fenetre_jeu, old_y, old_x, ' ');
						wattroff(fenetre_jeu, COLOR_PAIR(EAU_COLOR));*/
                        
					}else{
                        poisson->fuite = 5;
                    }

                    if(catch_up)    unlock(map, largeur, old_y-1, old_x);
					if(catch_down)  unlock(map, largeur, old_y+1, old_x);
					if(catch_right) unlock(map, largeur, old_y, old_x+1);
					if(catch_left)  unlock(map, largeur, old_y, old_x-1);

					catch_down = catch_left = catch_right = catch_up = 0;
					
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
                            case 'u': step(send_map, map, largeur, y, x, y-1, x); y--; break;
                            case 'd': step(send_map, map, largeur, y, x, y+1, x); y++; break;
                            case 'r': step(send_map, map, largeur, y, x, y, x+1); x++; break;
                            case 'l': step(send_map, map, largeur, y, x, y, x-1); x--; break;
                            default: exit(EXIT_FAILURE);
                        }
                        
                        /*sleep(1);*/
                        
                        
                        if(up) unlock(map, largeur, old_y-1, old_x);
                        if(down) unlock(map, largeur,old_y+1, old_x);
                        if(right) unlock(map, largeur,old_y, old_x+1);
                        if(left)unlock(map, largeur,old_y, old_x-1);

                        free(array);

                    }
					

                }
                unlock(map, largeur,old_y, old_x);
            break;
            }
            case EBUSY:   break;
            case EDEADLK: perror("Le mutex est lock et vérification d'erreur lui est signalé"); exit(EXIT_FAILURE); break;
        }

        up = down = right = left = 0;
        sleep(1);
    }

    return 0;
}