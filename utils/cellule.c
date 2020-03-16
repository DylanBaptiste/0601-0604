#include "cellule.h"

 /* Méthode initialisant une cellule à partir d'un entier  VIVE LA 501 */
cellule_t* initialiserCellule(unsigned short port , struct sockaddr_in adresse){
    cellule_t *cell = malloc(sizeof(*cell));
    if(cell == NULL){
        exit(EXIT_FAILURE);
    }
    cell->prochain = NULL;
    cell->IP = adresse.sin_addr;
    cell->PortTCP = port;
    return cell;
}

/* Méthode détruisant une cellule */
void detruireCellule(cellule_t* cell){
    if(cell == NULL){
        exit(EXIT_FAILURE);
    }
    free(cell);
}