#ifndef __CASE_H__
#define __CASE_H__

#include "poisson.h"
#include "joueur.h"
#include <stdlib.h>

#define VIDE 9
#define POISSON 1
#define PNEU 2
#define DYNAMITE 3
#define REQUIN 4
#define JOUEUR 5

typedef struct case_type{
    int element;
    pthread_mutex_t mutex;
    union{
        poisson_t* poisson;
        joueur_t* joueur;
    }data;
}case_t;

void* routine_poisson(void* arg);

#endif
