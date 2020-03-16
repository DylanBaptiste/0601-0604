#ifndef __CELLULE_H__
#define __CELLULE_H__

#include <stdlib.h>      /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>       /* Pour printf, fprintf, perror */
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include <string.h>      /* Pour memset */
#include <unistd.h>      /* Pour close */
#include <errno.h>       /* Pour errno */

typedef struct cellule_type {
    struct cellule_type * prochain;
    struct in_addr IP;
    unsigned short PortTCP;
} cellule_t;

cellule_t* initialiserCellule(unsigned short, struct sockaddr_in);
void detruireCellule(cellule_t*);

#endif
