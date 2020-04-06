#ifndef __POISSON_H__
#define __POISSON_H__

#include <pthread.h>
#include <stdlib.h>

#include "../config.h"

#define SPAWN_RATE_1 50
#define SPAWN_RATE_2 35
#define SPAWN_RATE_3 100 - ( SPAWN_RATE_2 + SPAWN_RATE_1 )

typedef struct poisson_type{
    int fuite;
    char valeur;
    int poireaus;
    pthread_mutex_t mutex;
}poisson_t;

poisson_t* generer_poisson(int seed);


#endif
