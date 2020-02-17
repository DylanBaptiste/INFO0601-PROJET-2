#include <string.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h> 
#include "ncurses_utils.h"
#include "file_utils.h"
#include <stdio.h> 

#include "config.h"

int nbV, fd = 0;
unsigned char map[ MAP_HAUTEUR * MAP_LARGEUR ];
int quitter = FALSE;
WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;

void placer_element(int y, int x, unsigned char c, bool write);


void handler(int s){
    printf("signal: %d\n", s);
}

int main(int argc, char** argv) {
    int i, j, k, startMenu;
    WINDOW *box_log, *box_jeu, *box_etat;

    if(argc != 3){
		printf("mauvaise utilisation: ./editeur [<path>] [<nombre voitures>]\n");
		exit(EXIT_FAILURE);
	}else{
		nbV = atoi(argv[2]);
        if(strcmp(getFileExt(argv[1]), "sim") == 0){
            fd = openFileSim(argv[1], map);
        }
        else{
            printf("la simulation doit etre un fichier .sim\n");
            exit(EXIT_FAILURE);
        }		
	}

    ncurses_initialiser();

	box_log  = create_box(&fenetre_log,  HAUTEUR1, LARGEUR1, POSY1, POSX1);
	box_jeu  = create_box(&fenetre_jeu,  HAUTEUR2, LARGEUR2, POSY2, POSX2);
	box_etat = create_box(&fenetre_etat, HAUTEUR3, LARGEUR3, POSY3, POSX3);
	
	mvwprintw(box_log,  0, 3, "Logs");
	mvwprintw(box_etat, 0, 3, "Etat");
	wrefresh(box_log );
	wrefresh(box_jeu );
	wrefresh(box_etat);

	scrollok(fenetre_log, TRUE);	
	
	for(i = 0, k = 0; i < MAP_HAUTEUR; i++){
		for(j = 0; j < MAP_LARGEUR; j++, k++){
			placer_element(i, j, map[k], false);
		}
	}
    wrefresh(fenetre_jeu );
    getch();
        
    startMenu = START_MENU;
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter:    F2");
    ++startMenu;
    mvwprintw(fenetre_etat, ++startMenu, 0, "Route:      +");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");


    wrefresh(fenetre_jeu);
    wrefresh(fenetre_etat);

    timeout(500);
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
            
    }


    ncurses_stopper();

    exit(EXIT_SUCCESS);
}


void placer_element(int y, int x, unsigned char c, bool write){
		
	switch (c)
	{
		case VIDE:
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