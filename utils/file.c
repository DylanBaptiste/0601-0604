#include "file.h"

file_t* initialiserFile(){
    file_t* file = (file_t*)malloc(sizeof(file_t));
    file->tete = NULL;
    file->queue = NULL;
    
    return file;
}

void detruire_file (file_t* file){
    free(file); 
}

int file_vide(file_t* file){
    return file->tete == file->queue)
}



void enfiler(file_t* file, cellule_t* cell){
   if (file->queue == NULL){
       file->queue = cell;
       file->tete = cell;
   }
   else {
       cell->prochain = file->queue;
       file->queue = cell;
   }
}

cellule_t * defiler(file_t* file){
    cellule_t* cell = NULL;
    
    if(file->tete != NULL){
        cell = file->tete;
        if (file->queue == file->tete){
            file->queue = NULL;
            file->tete = NULL;
        }
        else
        {
            while(file->queue != file->tete){
                file->queue = file->queue->prochain;
            }
            file->queue->prochain = NULL;
            file->tete = file->queue;
        }
    }
    
    return cell;
}

