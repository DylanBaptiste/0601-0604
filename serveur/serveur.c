#define _GNU_SOURCE
#include "utils/serveur_func.h"
#include "config.h"
#include "structures/case.h"


#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include <string.h>      /* Pour memset */



int noStopSignal = 0;
int GameEnCour = 0;

void handler(int sign){
  if (sign == SIGINT){
    noStopSignal = 1;
    GameEnCour = 1;
  }
}

int search_free_thread(){
	int i = 0;
	for(i = 0; i < NB_MAX_PARTY; i++){
		if(games[i].start == 0){
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[]){

    int sockUDP, sockTCP, i = 0;
  
    struct sockaddr_in adresseUDP;
    struct sigaction action;
    struct sockaddr_in Client1;
    struct sockaddr_in Client2;
    socklen_t socklen;
    
    uint16_t portTCP; /* Numero de port code sur 16 bits */
    int taillePartie[2];
    
    struct sockaddr_in adresseTCP;
    attr_r_partie* attr_game = malloc(sizeof(attr_r_partie));
	
	attr_game->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init((attr_game->mutex), NULL); /*permet de rendre sequentiel le lancement des threads pour la recuperation des attr*/
    

	for(i = 0; i < NB_MAX_PARTY; i++){
		games[i].start = 0;
		games[i].threads_TCP = (pthread_t*) malloc(sizeof(pthread_t));
	}
    
    /* Récupération des arguments */
    if(argc != 2) {
        fprintf(stderr, "Usage : %s port\n", argv[0]);
        fprintf(stderr, "Où :\n");
        fprintf(stderr, "  port           : port d'écoute du serveur\n");
        exit(EXIT_FAILURE);
    }

    socklen = sizeof(struct sockaddr_in);
    /* Signal pour l'arret */
    sigemptyset(&action.sa_mask);
    action.sa_handler = handler;
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    /*-----------------------------Partie UDP------------------*/
    /* Création de la socket */
    if((sockUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Erreur lors de la création de la socket ");
        exit(EXIT_FAILURE);
    }
    
    /* Création de l'adresse du serveur */
    memset(&adresseUDP, 0, sizeof(struct sockaddr_in));
    adresseUDP.sin_family = AF_INET;
    adresseUDP.sin_port = htons(atoi(argv[1]));
    adresseUDP.sin_addr.s_addr = htonl(INADDR_ANY);
   
    /* Nommage de la socket */
    if(bind(sockUDP, (struct sockaddr*)&adresseUDP, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur lors du nommage de la socket ");
        exit(EXIT_FAILURE);
    }


    while(!noStopSignal){

        /* Attente de la requête UDP des clients */
        printf("Serveur en attente du message UDP des clients.\n");
        memset(&Client2, 0, sizeof(struct sockaddr_in));
        memset(&Client1, 0, sizeof(struct sockaddr_in));

        /*le client 1 decide de la taille de la map*/
        if(recvfrom(sockUDP, &taillePartie, 2*sizeof(int), 0, (struct sockaddr *)&Client1, &socklen) == -1) {
            perror("Erreur lors de la réception du message ");
            exit(EXIT_FAILURE);
        }
        printf("Client 1 %d %d\nEn attente d'un deuxieme client...\n", taillePartie[0], taillePartie[1]);

        if(recvfrom(sockUDP, NULL, 0, 0, (struct sockaddr *)&Client2, &socklen) == -1) {
            perror("Erreur lors de la réception du message ");
            exit(EXIT_FAILURE);
        }
        printf("Client 2 trouvé.\n");
		printf("Lock %p\n", (void*)&attr_game->mutex);

		pthread_mutex_lock(attr_game->mutex);

		if( ( i = search_free_thread() ) == -1){
			printf("Il n'y a plus de place !\n");
			portTCP = 0;
			/*prevenir les client qu'ils n'y a plus de place*/
			if(sendto(sockUDP, &portTCP, sizeof(uint16_t), 0, (struct sockaddr*)&Client1, sizeof(struct sockaddr_in)) == -1) {
				perror("Erreur lors de l'envoi d'un message ");
			}
			if(sendto(sockUDP, &portTCP, sizeof(uint16_t), 0, (struct sockaddr*)&Client2, sizeof(struct sockaddr_in)) == -1) {
				perror("Erreur lors de l'envoi d'un message ");
			}
			printf("Les clients sont prevenu qu'il n'y a plus de place\n");
			pthread_mutex_unlock(attr_game->mutex);
		}else{
			printf("il y a de la place !\n");
			/* Création de la socket */
			if((sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
				perror("Erreur lors de la création de la socket ");
				exit(EXIT_FAILURE);
			}
			/* Création de l'adresse du serveur */
			memset(&adresseTCP, 0, sizeof(struct sockaddr_in));
			adresseTCP.sin_family = AF_INET;
			adresseTCP.sin_addr.s_addr = htonl(INADDR_ANY);
			adresseTCP.sin_port = htons((unsigned char)0); /*Prend un port aleatoire */
					
			/* Nommage de la socket */
			if(bind(sockTCP, (struct sockaddr*)&adresseTCP, sizeof(struct sockaddr_in)) == -1) {
				perror("Erreur lors du nommage de la socket ");
				exit(EXIT_FAILURE);
			}

			/* commmande pour voir nos port sous linux netstat -paunt */
			if(getsockname(sockTCP, (struct sockaddr*)&adresseTCP, &socklen) == -1) {
				perror("Erreur getsockname");
				exit(EXIT_FAILURE);
			}

			portTCP = ntohs(adresseTCP.sin_port);
			printf("Partie %d port: %d \n", i, portTCP);

			
			games[i].start = 1;
			/*attr_game = malloc(sizeof(attr_r_partie));*/
			attr_game->sockTCP = sockTCP;
			attr_game->hauteur = taillePartie[1];
			attr_game->largeur = taillePartie[0];
			attr_game->sockUDP = sockUDP;
			attr_game->portTCP = portTCP;
			attr_game->Client1 = Client1;
			attr_game->Client2 = Client2;
			attr_game->nb = i;
			pthread_create( games[i].threads_TCP, NULL, routine_partie, attr_game);
		}
        
    }

	free(attr_game);

    /* Fermeture de la socket UDP*/
    if(close(sockUDP) == -1) {
        perror("Erreur lors de la fermeture de la socket ");
        exit(EXIT_FAILURE);
    }

    printf("Serveur terminé.\n");

    exit(EXIT_SUCCESS);
}