#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEV 1

#define LARGEUR1 80
#define HAUTEUR1 5
#define POSX1    0
#define POSY1    0

#define LARGEUR2 (30 + 2) 
#define HAUTEUR2 (15 + 2)  
#define POSX2    POSX1
#define POSY2    HAUTEUR1

#define LARGEUR3 ( LARGEUR1 - LARGEUR2 )
#define HAUTEUR3 HAUTEUR2  
#define POSX3    LARGEUR2  
#define POSY3    HAUTEUR1  

#define START_MENU	2

#endif