#ifndef __PNEU_H__
#define __PNEU_H__

#include <pthread.h>

typedef struct pneu_type{
    int proprio;
    pthread_mutex_t mutex;
}pneu_t;

#endif
