#ifndef __FILE_H__
#define __FILE_H__

#include "cellule.h"
#include <stdlib.h>      /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>       /* Pour printf, fprintf, perror */
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include <string.h>      /* Pour memset */
#include <unistd.h>      /* Pour close */
#include <errno.h>       /* Pour errno */

typedef struct file_type {
    cellule_t* tete;
    cellule_t* queue;
    
} file_t;

file_t* initialiserFile();
void detruire_file(file_t*);
int file_vide(file_t*);
int file_pleine(file_t*);
void enfiler(file_t*, cellule_t*);
cellule_t* defiler(file_t*);

#endif