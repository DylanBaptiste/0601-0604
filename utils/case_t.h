#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define EAU 0
#define POISSONS 1
#define PNEU 2
#define DYNAMITE 3
#define REQUIN 4
#define JOUEUR1 5
#define JOUEUR2 6
/*
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_RemiseZero = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_Affichage = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_incrementation = PTHREAD_COND_INITIALIZER;
*/
typedef struct poisson_type{
    pthread_mutex_t mutex;
    //+ 1000 cond
    int valeur;
}poisson_t;

typedef struct requin_type{
    pthread_mutex_t mutex;
    int proprio;
}requin_t;

typedef struct pneu_type{
    pthread_mutex_t mutex;
    int proprio;
}pneu_t;

typedef struct dynamite_type{
    pthread_mutex_t mutex;
    //cond explosion
    int proprio;
}dynamite_t;

typedef struct case_type{
    int element;
    union{
        poisson_t poisson;
        requin_t requin;
        pneu_t pneu;
    }data;

}case_t;
