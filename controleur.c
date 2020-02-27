#include <string.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <stdio.h> 
#include <sys/shm.h>    /* Pour shmget, shmat, shmdt */
#include <errno.h>      /* Pour errno */
#include <sys/stat.h>   /* Pour S_IRUSR, S_IWUSR */
#include "utils/ncurses_utils.h"
#include "utils/file_utils.h"
#include "utils/config.h"
#define MESSAGE_SIZE 10

typedef struct voiture_type{
    pid_t pid;
    int numero;
    int y, x;
}voiture_t;


int nbV, fd = 0;
unsigned char* map;
int quitter = FALSE;
WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;
void placer_element(int y, int x, unsigned char c, bool write);
void handler(int s);
key_t SMP_key, MQ_key, SEM_key;
int SMP_id, MQ_id, SEM_id;
voiture_t* listeVoitures;
unsigned short* semvals;
pid_t* tableauPID;
/*int getVoitureIndex(pid_t pid);*/
void V(int semid, int s);

void P(int semid, int s);

bool try_P(int semid, int s);

int try_co(int SEM_id);

void clean_ncurses();
void clean_SEM();
void clean_MQ();
void clean_SMP();

int isExist(pid_t pid){
    int i = 0;
    for(i = 0; i < nbV; ++i){
        if(tableauPID[i] == pid){
            return i;
        }
    }
    return -1;
}

void ajouterPid(pid_t pid){
    int i;
    if(isExist(pid) == -1){
        for(i = 0; i < nbV; ++i){
            if(tableauPID[i] == pid){
                return i;
            }
        }
    }
}


int main(int argc, char** argv) {
/* ============ Variables locales ============ */
    int i, j, k, startMenu, oldX, oldY;
    WINDOW *box_log, *box_jeu, *box_etat;
	unsigned char titre[MAXFNAME];
    unsigned short semvals[2];
    requete_t requete;
    requete_t reponse;
    
    
/* ============ INIT ============ */
    signal(SIGINT, handler);/*Liberation memoire gerer si SIGINT recus*/
    /*struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = handler;

    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("Erreur lors du positionnement du gestionnaire ");
        exit(EXIT_FAILURE);
    }*/

    if(argc != 6){
		fprintf(stdout, "mauvaise utilisation: ./controleur [<path>] [<nombre voitures>] [<CLE de Segment Memoire Partagee>]  [<CLE de File Messages>] [<CLE Semaphores>]\n");
		exit(EXIT_FAILURE);
	}else{
		nbV           = atoi(argv[2]);
        SMP_key       = atoi(argv[3]);
        MQ_key        = atoi(argv[4]);
        SEM_key    = atoi(argv[5]);
        
        map = malloc( MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char) + 2 * sizeof(unsigned char) * nbV);
        tableauPID = malloc(sizeof(pid_t) * nbV);

        if(strcmp(getFileExt(argv[1]), "sim") == 0){
            fd = openFileSim(argv[1], map, titre);
            /*
            listeVoitures = malloc( nbV * sizeof(voiture_t));
            for(i = 0; i < nbV; ++i){
                listeVoitures[i].numero = i;
                listeVoitures[i].pid = -1;
                listeVoitures[i].y = -1;
                listeVoitures[i].x = -1;
            }
            */
           semvals[0] = nbV;
           semvals[1] = 1;
           /* semvals  = malloc( 2 * sizeof(unsigned short));
            semvals[0] = nbV;
            for(i = 1; i < 2-1; ++i){ semvals[i] = 1; }*/
        }
        else{
            fprintf(stderr, "la simulation doit etre un fichier .sim\n");
            exit(EXIT_FAILURE);
        }		
	}

/* ============ Mise en place des Semaphores ============ */

    if ((SEM_id = semget((key_t)SEM_key, 2, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1){
        if (errno == EEXIST)
            fprintf(stderr, "Tableau de semaphores (cle %d) deja existante\n", SEM_key);
        else
            perror("Erreur lors de la creation du tableau semaphores\n");
        exit(EXIT_FAILURE);
    }
    /*init sem*/
    if (semctl(SEM_id, 0, SETALL, semvals) == -1){
        perror("Erreur semctl SETALL\n");
        exit(EXIT_FAILURE);
    }

    if( atexit(clean_SEM) != 0) {
        perror("Probleme lors de l'enregistrement clean_SEM");
        exit(EXIT_FAILURE);
    }

/* ============ Mise en place du Segment de Memoire Partagee ============ */
    if((SMP_id = shmget(SMP_key, MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char) + 2 * sizeof(unsigned char) * nbV, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST) fprintf(stderr, "Le segment de mémoire partagée (cle=%d) existe deja\n", SMP_key);
        else perror("Erreur lors de la création du segment de mémoire ");
            ncurses_stopper(); /*coupure de ncurses*/

        exit(EXIT_FAILURE);
    }
    if( atexit(clean_SMP) != 0) {
        perror("Probleme lors de l'enregistrement clean_SMP");
        exit(EXIT_FAILURE);
    }
    if((map = (unsigned char*)shmat(SMP_id, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }
    readMap(fd, map, titre);
    for(i = MAP_HAUTEUR * MAP_LARGEUR; i < MAP_HAUTEUR * MAP_LARGEUR + 2 * nbV ; i ++ ){
        map[i]   = 255;
    }

    
    
/* ============mise en place de la file de messages ============ */
    if((MQ_id = msgget((key_t)MQ_key, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST)
            fprintf(stderr, "Erreur : file (cle=%d) existante\n", MQ_key);
        else
            perror("Erreur lors de la creation de la file ");
        exit(EXIT_FAILURE);
    }

    if( atexit(clean_MQ) != 0) {
        perror("Probleme lors de l'enregistrement clean_MQ");
        exit(EXIT_FAILURE);
    }

/* ============ Mise en place graphique ============ */
    
    ncurses_initialiser();

	box_log  = create_box(&fenetre_log,  HAUTEUR1, LARGEUR1, POSY1, POSX1);
	box_jeu  = create_box(&fenetre_jeu,  HAUTEUR2, LARGEUR2, POSY2, POSX2);
	box_etat = create_box(&fenetre_etat, HAUTEUR3, LARGEUR3, POSY3, POSX3);
    mvwprintw(box_jeu,  0, 3, "%s", (char*)titre);
	mvwprintw(box_log,  0, 3, "Logs");
	mvwprintw(box_etat, 0, 3, "Etat");
	wrefresh(box_log );
	wrefresh(box_jeu );
	wrefresh(box_etat);
	scrollok(fenetre_log, TRUE);	
    startMenu = START_MENU;
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter:    F2");
    ++startMenu;
    mvwprintw(fenetre_etat, ++startMenu, 0, "Obstacle:      +");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    wrefresh(fenetre_jeu);
    wrefresh(fenetre_etat);
    wrefresh(fenetre_log);

    if(atexit(clean_ncurses) != 0) {
        perror("Probleme lors de l'enregistrement clean_ncurses");
       exit(EXIT_FAILURE);
    }
    for(i = 0, k = 0; i < MAP_HAUTEUR; i++){
		for(j = 0; j < MAP_LARGEUR; j++, k++){
			placer_element(i, j, map[k], false);
		}
	}

/* ============ Boucle de jeu ============ */
    
    
    timeout(500);

    while(getch() != KEY_F(2) || quitter == FALSE){

        wrefresh(fenetre_jeu);
        wrefresh(fenetre_etat);
        wrefresh(fenetre_log);

        if(msgrcv(MQ_id, &requete, sizeof(requete_t) - sizeof(long), -4, 0) == -1) {

            /*if (errno == EINTR){
                wprintw(fenetre_log, "signal recu lors de l'attente d'un message");
                quitter = true;
            }
            if (errno == -1 ){
                perror("Erreur lors de la réception d'une requête ");
                exit( EXIT_FAILURE);
            }*/
            perror("Erreur lors de la réception d'une requête ");
            exit(EXIT_FAILURE);
        }
        else{
            switch(requete.type){
                case TYPE_CONFIG:
                    wprintw(fenetre_log, "Reception: Get CONFIG\n");
                    
                    /*Envoie de la reponse*/                
                    reponse.type = TYPE_SEND;
                    reponse.data.SendConfig.cle_sem = SEM_key;
                    reponse.data.SendConfig.cle_smp = SMP_key;
                    wprintw(fenetre_log, "id: %d key: %d\n", SEM_id, SEM_key);
                    wrefresh(fenetre_log);

                    if(msgsnd(MQ_id, &reponse, sizeof(requete_t) - sizeof(long), 0) == -1) {
                        perror("Erreur lors de l'envoi de la reponse ");
                        exit(EXIT_FAILURE);
                    }
                    break;

                case TYPE_MODIFCARTE:
                    wprintw(fenetre_log, "Reception: MODIFCARTE\n");
                    wrefresh(fenetre_jeu);
                    wrefresh(fenetre_log);
                    break;

                case TYPE_COREUSSIE:
                    wprintw(fenetre_log, "Reception: CO REUSSIE\n");
                    tableauPID[requete.data.ConnexionReussie.identifiant] = requete.data.ConnexionReussie.pid;
                    break;

                case TYPE_DECO:
                    wprintw(fenetre_log, "Reception: DECO\n");
                    tableauPID[requete.data.Deco.identifiant] = 0;
                    break;

                default:
                    wprintw(fenetre_log, "Type inconnu recu\n");
                    wrefresh(fenetre_log);
                    break;
            }
        }
    }

/* ============ Liberation memoire grace au atexit ============ */

    exit(EXIT_SUCCESS);
}
void clean_SEM(){
    fprintf(stdout, "clean_SEM\n");
    /* Suppression du tableau de semaphore */
    if(semctl(SEM_id, 0, IPC_RMID) == -1) {
        perror("Erreur lors de la suppresion du tableau de semaphores SEM_id");
    }
}
void clean_MQ(){
    fprintf(stdout, "clean_MQ\n");

   /* Suppression de la file */
    if(msgctl(MQ_id, IPC_RMID, 0) == -1) {
        perror("Erreur lors de la suppression de la file");
    }
}

void clean_SMP(){
    fprintf(stdout, "clean_SMP\n");

    /* Suppression du segment de mémoire partagée */
    if(shmctl(SMP_id, IPC_RMID, 0) == -1) {
        perror("Erreur lors de la suppression du segment de mémoire partagée");
    }
}


void clean_ncurses(){
    ncurses_stopper();
    fprintf(stdout, "clean_ncurses\n");
    /*et delete fenetre*/
}

void handler(int s){
    quitter = s == SIGINT;
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

int try_co(int SEM_id){
    /*struct semid_ds sem_buf;
    int i;*/

    /* Recuperation des informations sur le tableau de semaphores */
    /*if(semctl(SEM_id, 0, IPC_STAT, &sem_buf) == -1) {
        perror("Erreur lors de la recuperation d'info sur le tableau de semaphores ");
        exit(EXIT_FAILURE);
    }*/

    /*Tentative de lock 1 sem*/
    /*for(i = 0; i < sem_buf.sem_nsems; ++i){
        if(try_P(SEM_id, i)){
            return i;
        }
    }
    return -1;*/
    return try_P(SEM_id, 0) == true;
}
/*
int getVoitureIndex(pid_t pid){
    for (size_t i = 0; i < nbV; i++)
    {
        if(listeVoitures[i].pid == pid){
            return i;
        }
    }
    
    return -1;
}
*/
void placer_element(int y, int x, unsigned char c, bool write){	
    switch ((unsigned int)c)
	{
		case MUR:
			mvwprintw(fenetre_jeu, y, x, "+");
            map[MAP_LARGEUR*y+x] = MUR;
			break;
		case ROUTE :
			mvwprintw(fenetre_jeu, y, x, " ");
            map[MAP_LARGEUR*y+x] = ROUTE;
			break;
	
		default:
            if(c > nbV + 2){
                mvwprintw(fenetre_jeu, y, x, "?");
            }else{
                mvwprintw(fenetre_jeu, y, x, "V");
            }
            map[MAP_LARGEUR*y+x] = c;

			break;
	}
    wrefresh(fenetre_jeu);
	if( write == true){
		insertElement(fd, y, x, c);
	}
	
}

