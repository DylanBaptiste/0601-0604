#include "poisson.h"

poisson_t* generer_poisson(int seed){
    int r;
    poisson_t* p;
    srand(time(0) + seed);
    r = rand()%100;
    p = malloc(sizeof(poisson_t));
    p->fuite = 0;
    p->valeur = r < SPAWN_RATE_1 ? '1' : ( r < SPAWN_RATE_1 + SPAWN_RATE_2 ? '2' : '3' );
    switch (p->valeur)
    {
        case '1': p->poireaus = 100; break;
        case '2': p->poireaus = 200; break;
        case '3': p->poireaus = 500; break;
        default:  p->poireaus = 100; break;
    }
    p->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    return p;
}
