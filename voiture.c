#include <string.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>    /* Pour shmget, shmat, shmdt */
#include <errno.h>      /* Pour errno */
#include <sys/stat.h>   /* Pour S_IRUSR, S_IWUSR */
#include "utils/ncurses_utils.h"
#include "utils/file_utils.h"
#include "utils/config.h"

int map[ MAP_HAUTEUR * MAP_LARGEUR ];
key_t CLE_SMP = 0;

typedef struct message{
    __pid_t pid;
    char message[10];
} message_t;

int shmid;
message_t *adresse;

void handler(int s){
    printf("signal: %d\n", s);
}

int main(int argc, char** argv) {
    /* Création d'un segment de 10 entiers */
    if((shmid = shmget((key_t)CLE_SMP, sizeof(unsigned char) * MAP_HAUTEUR * MAP_LARGEUR, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST){
            fprintf(stderr, "Le segment de mémoire partagée (cle=%d) existe deja\n", CLE_SMP);
        
            /* Récupération du segment de mémoire partagée */
            if((shmid = shmget((key_t)CLE_SMP, 0, 0)) == -1) {
                perror("Erreur lors de la récupération du segment de mémoire partagée ");
                exit(EXIT_FAILURE);
            }
            printf("Client : récupération du segment de mémoire partagée.\n");
            
            /* Attachement du segment de mémoire partagée */
            if((adresse = shmat(shmid, NULL, 0)) == (void*)-1) {
                perror("Erreur lors de l'attachement du segment de mémoire partagée ");
                exit(EXIT_FAILURE);
            }

            /* Lecture du message dans le segment de mémoire partagée */
            printf("Mesage lu:  %d %s", (int)adresse->pid, adresse->message);
                

            /* Détachement du segment de mémoire partagée */
            if(shmdt(adresse) == -1) {
                perror("Erreur lors du détachement du segment de mémoire partagée ");
                exit(EXIT_FAILURE);
            }
            
            return EXIT_SUCCESS;
        }
        else
            perror("Erreur lors de la création du segment de mémoire ");
        
        exit(EXIT_FAILURE);
    }
    printf("Serveur : segment crée.\n");

    /* Attachement du segment de mémoire partagée */
    if((adresse = shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }

    
    /* Détachement du segment de mémoire partagée */
    if(shmdt(adresse) == -1) {
        perror("Erreur lors du détachement ");
        exit(EXIT_FAILURE);
    }







     exit(EXIT_SUCCESS);
		
}
