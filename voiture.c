#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
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
requete_t requete;

void libererPlace();
void deconnexion();

void handler(int s){
    fprintf(stdout, "\nSignal recu: %d\n", s);
    quitter = s == SIGINT;
}



void detachementSMP();

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
    int i, identifiant = 0;
    for (i = MAP_HAUTEUR * MAP_LARGEUR + 1; i < taille; i+=2)
    {
        if (map[i] != 255)
        {
            identifiant++;
        }
        
    }
    return identifiant;
}

int main(int argc, char** argv) {
    struct shmid_ds smp_info;
    int old_x, old_y;
    /*struct timespec ts;*/
    /*struct timespec tsRand;*/


    
/*map = malloc( MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char));*/
    signal(SIGINT, handler); /*Liberation memoire gerer si SIGINT recus*/
    MQ_key = atoi(argv[1]);
    /*ts.tv_sec = 0;
    ts.tv_nsec = (long)atoi(argv[2]);*/
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

    /*atexit liberer sa place*/
    if(atexit(libererPlace) != 0) {
        perror("Probleme lors de l'enregistrement libererPlace");
       exit(EXIT_FAILURE);
    }

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

    /*atexit detachement du segment de memoire partagé*/
    if(atexit(detachementSMP) != 0) {
        perror("Probleme lors de l'enregistrement detachementSMP");
       exit(EXIT_FAILURE);
    }

    P(SEM_id, 1);
    
    /*recupêraton des infos du SMP*/
    if( shmctl(SMP_id, IPC_STAT, &smp_info) == -1) {
        perror("Erreur shmctl SMP IPC_STAT");
        exit(EXIT_FAILURE);
    }

    identifiant = getIdentifiant(smp_info.shm_segsz);
    fprintf(stdout, "Mon id est %d\n", identifiant);

    /*on previent le controleur d'enregistrer notre pid*/
    requete.type = TYPE_COREUSSIE;
    requete.data.ConnexionReussie.pid = getpid();
    requete.data.ConnexionReussie.identifiant = identifiant;

    fprintf(stdout, "Avertisssement au controleur de ma connexion...\n");
    if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête Modif");
        exit(EXIT_FAILURE);
    }

    if(atexit(deconnexion) != 0) {
        perror("Probleme lors de l'enregistrement deconnexion");
       exit(EXIT_FAILURE);
    }
    
    srand(time(NULL) * (int)getpid());

    do{
        y = randomRange(0, MAP_HAUTEUR);
        x = randomRange(0, MAP_LARGEUR);
    }while(!is_free(y, x));
    
    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant]     = x;
    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant + 1] = y;

    requete.type = TYPE_MODIFCARTE;
    requete.data.ModifCarte.pid = getpid();
    requete.data.ModifCarte.old_x = x;
    requete.data.ModifCarte.old_y = y;
    requete.data.ModifCarte.x = x;
    requete.data.ModifCarte.y = y;
    requete.data.ModifCarte.identifiant = identifiant;

    if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête Modif");
        exit(EXIT_FAILURE);
    }

    V(SEM_id, 1);
    while (quitter == FALSE)
    {
        
        old_x = x;
        old_y = y;
        
        direction = randomRange(0, 3);
        switch (direction)
        {
            case 0:
                P(SEM_id, 1);
                if( y < MAP_HAUTEUR && is_free(y+1, x)){
                    map[MAP_LARGEUR*y+x] = ROUTE;
                    y++;
                    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant + 1] = y;
                    map[MAP_LARGEUR*y+x] = identifiant + 2;
                }
                else{
                    direction = (int)(rand() % 4);
                }
                V(SEM_id, 1);
                break;
            case 1:
                P(SEM_id, 1);
                if( y > 0 && is_free(y-1, x)){
                    map[MAP_LARGEUR*y+x] = ROUTE;
                    y--;
                    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant + 1] = y;
                    map[MAP_LARGEUR*y+x] = identifiant + 2;
                }
                else{
                    direction = (int)(rand() % 4);
                }
                V(SEM_id, 1);
                break;
            case 2:
                P(SEM_id, 1);
                if( x < MAP_LARGEUR && is_free(y, x+1)){
                    map[MAP_LARGEUR*y+x] = ROUTE;
                    x++;
                    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant] = x;
                    map[MAP_LARGEUR*y+x] = identifiant + 2;
                }
                else{
                    direction = (int)(rand() % 4);
                }
                V(SEM_id, 1);
                break;
            case 3:
                P(SEM_id, 1);
                if( x > 0 && is_free(y, x-1)){
                    map[MAP_LARGEUR*y+x] = ROUTE;
                    x--;
                    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant] = x;
                    map[MAP_LARGEUR*y+x] = identifiant + 2;
                }
                else{
                    direction = (int)(rand() % 4);
                }
                V(SEM_id, 1);
                break;
            
            default:
                fprintf(stdout, "?%d?\n", direction);

                break;
        }
        
        fprintf(stdout, "%d", direction);

        /*fprintf(stdout, "(%d, %d)", y, x);*/
        fflush(stdout);

        if(old_x != x || old_y != y){
            requete.type = TYPE_MODIFCARTE;
            requete.data.ModifCarte.pid = getpid();
            requete.data.ModifCarte.old_x = old_x;
            requete.data.ModifCarte.old_y = old_y;
            requete.data.ModifCarte.x = x;
            requete.data.ModifCarte.y = y;
            requete.data.ModifCarte.identifiant = identifiant;

            if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
                perror("Erreur lors de l'envoi de la requête Modif");
                exit(EXIT_FAILURE);
            }
        }
        
        
        /*nanosleep (&ts, NULL);*/
        /*sleep(1);*/
        usleep(50000);
    }


    exit(EXIT_SUCCESS);
		
}
void deconnexion(){

    map[MAP_LARGEUR*y+x] = ROUTE;
    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant]     = 255;
    map[MAP_HAUTEUR * MAP_LARGEUR + 2 * identifiant + 1] = 255;

    /* Envoi de la requete de deconnexion */
    requete.type = TYPE_DECO;
    requete.data.Deco.identifiant = identifiant;
    requete.data.Deco.x = x;
    requete.data.Deco.y = y;
    requete.data.Deco.pid = getpid();

    fprintf(stdout, "Tentative de deconnexion...\n");
    if(msgsnd(MQ_id, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête ");
    }
}

void libererPlace(){
    struct semid_ds sem_buf;
    /* Recuperation des informations sur le tableau de semaphores */
    if(semctl(SEM_id, 0, IPC_STAT, &sem_buf) == -1) {
        switch (errno)
        {
        case EACCES:
            perror("Impossible d'avoir les stats");
            exit(EXIT_FAILURE);
            break;
        case EFAULT:
            perror("sem_buf pointe en-dehors de l'espace d'adressage accessible");
            exit(EXIT_FAILURE);
            break;
        case EIDRM:
            /*perror("pointe sur un segment détruit.");*/
            /**/
            break;
        case EINVAL:
            /*perror("Mauvais ID");
            exit(EXIT_FAILURE);*/
            break;
        case EOVERFLOW:
            perror("valeur de GID ou d'UID est trop grande pour être stockée dans la structure pointée par buf");
            exit(EXIT_FAILURE);
            break;
        
        default:
            break;
        }
    }else{
        /*liberation de la place*/
        fprintf(stdout, "Je libere ma place");
        V(SEM_id, 0);
    }
    
}

void detachementSMP(){
    /* Detachement du segment de memoire partagee */
    if(shmdt(map) == -1) {
        perror("Erreur lors du detachement SMP");
        exit(EXIT_FAILURE);
    }
}