#ifndef __FILEMESSAGE_H__
#define __FILEMESSAGE_H__
#define _XOPEN_SOURCE

#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>     
#include <stdio.h>      
#include <sys/msg.h>    
#include <errno.h>      
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include  <sys/types.h>
#include  <signal.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>


#define MAX_VROUMVROUM 5

#define TYPE_CONFIG 1
#define TYPE_SEND 2
#define TYPE_MODIFCARTE 3

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
    int voiture;
    char message[256];
}ModifCarte_t;


/* Structure Union le type en renvoie doit etre le pid (controleur -> Voiture) */
typedef struct requete_type{
    long type; 
    union 
    {
        RecupConfig_t RecupConfig;
        sendConfig_t SendConfig;
        ModifCarte_t ModifCarte;
    }data;
}requete_t;

int CreationFile();
void ReceptionFileConfig(int);
void ReponseFile(int,pid_t);
void SupprimerFile(int);
void handlerSignal(int);
#endif