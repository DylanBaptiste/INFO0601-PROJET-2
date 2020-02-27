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

#define SIM_MODE 0
#define DEC_MODE 1

#define DEFAULT_POSITION 255

#define MAXFNAME LARGEUR2 - 6

#define MAP_HAUTEUR (HAUTEUR2 - 2)
#define MAP_LARGEUR (LARGEUR2 - 2)

#define MUR 0
#define ROUTE 1



#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include  <sys/types.h>
#include  <signal.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>


#define TYPE_CONFIG 1
#define TYPE_MODIFCARTE 2
#define TYPE_COREUSSIE 3
#define TYPE_DECO 4
#define TYPE_SEND 5

/* Structure de connexion au controleur */
typedef struct RecupConfig_type{
    pid_t pid;
}RecupConfig_t;

/* Structure pour l'envoie des infos à la voiture */
typedef struct sendConfig_type{
    key_t cle_sem;
    key_t cle_smp;
}sendConfig_t;


/* Structure pour la modification de la carte au controleur */
typedef struct ModifCarte_type{
    pid_t pid;
    int y, x, identifiant;
}ModifCarte_t;

/* Structure pour la notification de connexion reussie */
typedef struct ConnexionReussie_type{
    pid_t pid;
    int identifiant;
}ConnexionReussie_t;

/* Structure pour la notification de deconnexion */
typedef struct Deco_type{
    pid_t pid;
    int identifiant;
}Deco_t;

/* Structure Union le type en renvoie doit etre le pid (controleur -> Voiture) */
typedef struct requete_type{
    long type; 
    union 
    {
        RecupConfig_t RecupConfig;
        sendConfig_t SendConfig;
        ModifCarte_t ModifCarte;
        ConnexionReussie_t ConnexionReussie;
        Deco_t Deco;
    }data;
}requete_t;

#endif