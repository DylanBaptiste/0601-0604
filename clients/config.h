#ifndef __CONFIG_H__
#define __CONFIG_H__

#define GENERAL_X 0
#define GENERAL_Y 0

#define LARGEUR_LOG 26
#define LARGEUR_ETAT 18
#define LARGEUR_MAGASIN 29
#define HAUTEUR_TOP 15

#define POSX_ETAT GENERAL_X  
#define POSY_ETAT GENERAL_Y

#define POSX_MAGASIN POSX_ETAT + LARGEUR_ETAT
#define POSY_MAGASIN GENERAL_Y

#define POSX_LOG POSX_ETAT + LARGEUR_ETAT + LARGEUR_MAGASIN  
#define POSY_LOG GENERAL_Y

#define POSX_ETANG GENERAL_X
#define POSY_ETANG HAUTEUR_TOP

#define NAME_ETANG " - - Etang - - "
#define NAME_ETAT " - Etat - "
#define NAME_MAGASIN " - Magasin - "
#define NAME_LOG " - Logs - "

#define START_MENU	3



#endif
