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



int nbV, fd = 0;
unsigned char* map;
int quitter = FALSE;
WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;
void placer_element(int y, int x, unsigned char c, bool write);
void handler(int s);
key_t CLE_SMP;
int shmid;

int main(int argc, char** argv) {
/* ============ Variables locales ============ */
    int i, j, k, startMenu;
    WINDOW *box_log, *box_jeu, *box_etat;
	unsigned char titre[MAXFNAME];
    map = malloc( MAP_HAUTEUR * MAP_LARGEUR * sizeof(unsigned char));
    
/* ============ INIT ============ */
    if(argc != 4){
		fprintf(stdout, "mauvaise utilisation: ./editeur [<path>] [<nombre voitures>] [<CLE de Segment Memoire Partagee>]\n");
		exit(EXIT_FAILURE);
	}else{
		nbV     = atoi(argv[2]);
        CLE_SMP = atoi(argv[3]);
        if(strcmp(getFileExt(argv[1]), "sim") == 0){
            fd = openFileSim(argv[1], map, titre);
        }
        else{
            fprintf(stderr, "la simulation doit etre un fichier .sim\n");
            exit(EXIT_FAILURE);
        }		
	}

/* ============ Mise en place du Segment de Memoire Partagee ============ */
    /* Création d'un segment de  MAP_HAUTEUR * MAP_LARGEUR  unsigned char */
    if((shmid = shmget(CLE_SMP, sizeof(unsigned char) *  MAP_HAUTEUR * MAP_LARGEUR, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST) fprintf(stderr, "Le segment de mémoire partagée (cle=%d) existe deja\n", CLE_SMP);
        else perror("Erreur lors de la création du segment de mémoire ");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Controleur: segment crée.\n");

    /* Attachement du segment de mémoire partagée */
    if((map = shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }

    /* Placement des entiers dans le segment de mémoire partagée */
    for(i = 0, k = 0; i < MAP_HAUTEUR; i++){
		for(j = 0; j < MAP_LARGEUR; j++, k++){
			placer_element(i, j, map[k], false);
		}
	}
    fprintf(stdout, "Controleur: map placée dans le segment.\n");


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

/* ============ Boucle de jeu ============ */
    timeout(500);
    signal(SIGINT, handler);
    while(quitter == FALSE) {

        switch (getch())
        {
            case KEY_F(2):
                quitter = TRUE;
                break;

            default:
                /*generation();*/
                break;
        }
        
        wrefresh(fenetre_jeu);
        wrefresh(fenetre_etat);
        wrefresh(fenetre_log);
            
    }

/* ============ Liberation memoire ============ */
    
    /* Suppression du segment de mémoire partagée */
    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("Erreur lors de la suppression du segment de mémoire partagée ");
        exit(EXIT_FAILURE);
    }
    
    ncurses_stopper(); /*coupure de ncurses*/

    exit(EXIT_SUCCESS);
}

void handler(int s){
    quitter = s == SIGINT;
}

void placer_element(int y, int x, unsigned char c, bool write){	
	switch (c)
	{
		case MUR:
			mvwprintw(fenetre_jeu, y, x, "+");
			break;
		case ROUTE :
			mvwprintw(fenetre_jeu, y, x, " ");
			break;
	
		default:
            if(c > nbV + 2){
                mvwprintw(fenetre_jeu, y, x, "?");

            }else{
                mvwprintw(fenetre_jeu, y, x, "V");
            }
			break;
	}

	if( write == true){
		insertElement(fd, y, x, c);
	}
	
}

