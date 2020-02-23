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

unsigned char* map;
key_t CLE_SMP;

int shmid;


void handler(int s){
    printf("signal: %d\n", s);
}

int main(int argc, char** argv) {

    int i, j, k;

    map = malloc( MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char));
    CLE_SMP = atoi(argv[1]);
    
    
    /*envoie de la demande, recuperation*/
    
    
    
    
    /* Récupération du segment de mémoire partagée */
    if((shmid = shmget((key_t)CLE_SMP, 0, 0)) == -1) {
        perror("Erreur lors de la récupération du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }
    printf("Client : récupération du segment de mémoire partagée.\n");
    
    /* Attachement du segment de mémoire partagée */
    if((map = shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }
    /* Lecture de la map dans le segment de mémoire partagée */
    while(1){
        sleep(1);
        for(i = 0, k = 0; i < MAP_HAUTEUR; i++){
            for(j = 0; j < MAP_LARGEUR; j++, k++){
                printf("%d", map[k]);
            }
        }
        printf("\n");
    }
    /* Détachement du segment de mémoire partagée */
    if(shmdt(map) == -1) {
        perror("Erreur lors du détachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }


     exit(EXIT_SUCCESS);
		
}
