#ifndef __JOUEUR_H__
#define __JOUEUR_H__

#include <pthread.h>

typedef struct joueur_type{
    int number;
    int poireaus;
    int mode_furtif;
	pthread_mutex_t mutex;
    pthread_cond_t releve;
}joueur_t;

#endif
