
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


WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;

int main(int argc, char** argv) {

    

    if(argc != 5){
        fprintf(stdout, "mauvaise utilisation: ./client [<IP>] [<PORT>] [<HAUTEUR>] [<LARGEUR>]\n");
		exit(EXIT_FAILURE);
    }else{
        //atoi
    }

    //attend 2 reuqte UDP
    //dirige vers un port tcp

    exit(1);
}