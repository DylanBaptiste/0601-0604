#ifndef __SERVEUR_FUNC_H__
#define __SERVEUR_FUNC_H__

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <errno.h> 
#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include "../structures/case.h"



typedef struct send_map_t
{
    int element;
    char c;
}send_map_t;

typedef struct attr_r_poisson{
    int hauteur, largeur, number, game;
    case_t* map;
    send_map_t* send_map;
}attr_r_poisson;



typedef struct coord_t{
    int x, y, largeur;
    case_t* map;
}coord_t;

typedef struct attr_r_partie{
    int sockTCP, nb, largeur, hauteur, sockUDP;
    uint16_t portTCP;
    struct sockaddr_in Client1, Client2;
    pthread_mutex_t* mutex;
}attr_r_partie;

void* routine_lock(void* arg);

typedef struct control_thread
{
	int start;
	pthread_t* threads_TCP;

}control_thread_t;

control_thread_t games[NB_MAX_PARTY];

void unlock(case_t* map, int largeur, int y, int x);
void* routine_partie(void* arg);




#endif