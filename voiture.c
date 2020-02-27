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
key_t SMP_key, MQ_key, SEM_key;
int SMP_id, MQ_id, SEM_id;
int y, x;
int identifiant, direction;
int quitter = false;

void handler(int s){
    quitter = s == SIGINT;
}


void P(int semid, int s)
{
    struct sembuf op;

    op.sem_num = s;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1)
    {
        perror("Erreur P(s)\n");
        exit(EXIT_FAILURE);
    }
}

void V(int semid, int s)
{
    struct sembuf op;

    op.sem_num = s;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1)
    {
        perror("Erreur V(s)\n");
        exit(EXIT_FAILURE);
    }
}

bool try_P(int semid, int s)
{
    struct sembuf op;

    op.sem_num = s;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if (semop(semid, &op, 1) == -1)
    {
        if (errno == EAGAIN)
            return false;
        else{
            perror("Erreur try_P(s)\n");
            exit(EXIT_FAILURE);
        }
    }
    return true;
}

int is_free(int y, int x){ return map[MAP_LARGEUR*y+x] == ROUTE; }

int randomRange(int min, int max){
    return (rand() % (max - min + 1)) + min; 
}

int getIdentifiant(taille){
    int i, id_voiture = 0;
    for (i = 450; i < taille; i+=2)
    {
        if (map[i] != 255)
        {
            id_voiture++;
        }
        
    }
    return id_voiture;
}

int main(int argc, char** argv) {
    requete_t requete;
    int i, j, k;
    struct shmid_ds smp_info;
    /*map = malloc( MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char));*/
    signal(SIGINT, handler); /*Liberation memoire gerer si SIGINT recus*/
    MQ_key = atoi(argv[1]);
    
    /* Récupération de la file */
    if((MQ_id = msgget((key_t)MQ_key, 0)) == -1) {
        perror("Erreur lors de la récupération de la file ");
        exit(EXIT_FAILURE);
    }

    /* Envoi de la requete de connection */
    requete.type = TYPE_CONFIG;
    requete.data.RecupConfig.pid = getpid();
    fprintf(stdout, "Requete de connection...\n");
    if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête ");
        exit(EXIT_FAILURE);
    }

    /* Réception de la réponse */
    if(msgrcv(MQ_id, &requete, sizeof(requete_t) - sizeof(long), TYPE_SEND, 0) == -1) {
        perror("Erreur lors de la réception de la réponse ");
        exit(EXIT_FAILURE);    
    }
    fprintf(stdout, "Reponse recu: \nclef sem: %d\nclef smp: %d\n\n", requete.data.SendConfig.cle_sem, requete.data.SendConfig.cle_smp);
    
    SMP_key = requete.data.SendConfig.cle_smp;
    SEM_key = requete.data.SendConfig.cle_sem;

    if((SEM_id = semget((key_t)SEM_key, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du tableau de semaphores ");
        exit(EXIT_FAILURE);
    }

    printf("id: %d key: %d\n", SEM_id, SEM_key);

    fprintf(stdout, "Tentative de connection...\n");
    P(SEM_id, 0);
    fprintf(stdout, "Connexion reussi...\n");
    
    /* Récupération du segment de mémoire partagée */
    if((SMP_id = shmget((key_t)SMP_key, 0, 0)) == -1) {
        perror("Erreur lors de la récupération du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }
    printf("Client : récupération du segment de mémoire partagée.\n");
    
    /* Attachement du segment de mémoire partagée */
    if((map = shmat(SMP_id, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }

    if( shmctl(SMP_id, IPC_STAT, &smp_info) == -1) {
        perror("Erreur shmctl");
        exit(EXIT_FAILURE);
    }

    P(SEM_id, 1);
    
    identifiant = getIdentifiant(smp_info.shm_segsz);
    
    srand(time(0)); 
    do{
        y = randomRange(0, MAP_HAUTEUR);
        x = randomRange(0, MAP_LARGEUR);
    }while(!is_free(y, x));

    V(SEM_id, 1);

    /* Lecture de la map dans le segment de mémoire partagée */
    for(i = 0, k = 0; i < MAP_HAUTEUR; i++){
        for(j = 0; j < MAP_LARGEUR; j++, k++){
            printf("%d", map[k]);
        }
    }
    
    direction = (int)(rand() % 4);

    while (quitter == FALSE)
    {
        P(SEM_id, 1);
        /* Envoi de la requete de modifiaction */
        switch (direction)
        {
        case 0:
            /* code */
            break;
        
        default:
            break;
        }
        requete.type = TYPE_MODIFCARTE;
        requete.data.ModifCarte.pid = getpid();
        requete.data.ModifCarte.x = x++;
        requete.data.ModifCarte.y = y++;
        requete.data.ModifCarte.identifiant = identifiant;

        fprintf(stdout, "Modif envoye\n");
        if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
            perror("Erreur lors de l'envoi de la requête ");
            exit(EXIT_FAILURE);
        }
        V(SEM_id, 1);
        
        sleep(2);
    }
    
    P(SEM_id, 1);

    V(SEM_id, 1);

    /*liberation de la place*/
    V(SEM_id, 0);

    /* Envoi de la requete de deconnexion */
    requete.type = TYPE_DECO;
    requete.data.Deco.identifiant = identifiant;
    requete.data.Deco.pid = getpid();

    fprintf(stdout, "Tentative de deconnexion...\n");
    if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête ");
        exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);
		
}
