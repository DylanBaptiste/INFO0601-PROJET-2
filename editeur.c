#include <string.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>

#include "utils/ncurses_utils.h"
#include "utils/file_utils.h"
#include "utils/config.h"


int mWidth, mHeight, fd, mode;
unsigned char restartX, restartY, nbFlocon;
unsigned char** matrice;
WINDOW *fenetre_log, *fenetre_jeu, *fenetre_etat;


void placer_element(int y, int x, unsigned char c, bool write);
int is_free(int y, int x);
void generation();

int main(int argc, char** argv) {
	
	int i, j, k, startMenu, sourisX, sourisY, bouton;
	int quitter = FALSE;
	WINDOW *box_jeu, *box_etat, *box_log;
	unsigned char* mapBuffer = malloc((LARGEUR2 - 2)*(HAUTEUR2 - 2) + 1 * sizeof(unsigned char));
	if(mapBuffer == NULL){
        perror("erreur lors du malloc ");
        exit(EXIT_FAILURE);
    }
	
	

	fenetre_log  = NULL;
	fenetre_jeu  = NULL;
	fenetre_etat = NULL;

	mWidth  = LARGEUR2 - 2;
	mHeight = HAUTEUR2 - 2;
	matrice = malloc(mHeight * sizeof( unsigned char*));

	if(matrice == NULL){
        perror("erreur lors du malloc ");
        exit(EXIT_FAILURE);
    }
	for(i = 0; i < mWidth; i++){
		matrice[i] = malloc(mWidth * sizeof(unsigned char));
		if(matrice[i] == NULL){
			perror("erreur lors du malloc ");
			exit(EXIT_FAILURE);
    	}
	}

	
	if(argc != 2){
		printf("mauvaise utilisation: ./editeur [<path>]\n");
		exit(EXIT_FAILURE);
	}else{
		
        if(strcmp(getFileExt(argv[1]), "bin") == 0){
            fd = openFile(argv[1]);
            if (lseek(fd, 0, SEEK_SET) == -1 ){
                perror("Erreur Lors du LSEEK main");
                exit(EXIT_FAILURE);
            }
            
            readMap(fd, mapBuffer);
            mode = DEC_MODE;
        }else{
            printf("le decor doit etre un fichier .bin\n");
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
	
	for(i = 0, k = 0; i < mHeight; i++){
		for(j = 0; j < mWidth; j++, k++){
			placer_element(i, j, mapBuffer[k], false);
		}
	}
	

	

    startMenu = START_MENU;
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    mvwprintw(fenetre_etat, ++startMenu, 0, "Quitter:    F2");
    ++startMenu;
    mvwprintw(fenetre_etat, ++startMenu, 0, "Route:   +");
    mvwprintw(fenetre_etat, ++startMenu, 0, "-------");
    wrefresh(fenetre_etat);

    ncurses_souris();

    wrefresh(fenetre_jeu);
    wrefresh(box_jeu);
    
    while(quitter == FALSE) {
        i = getch();
        if( (int)i == KEY_MOUSE){
            if(souris_getpos(&sourisX, &sourisY, &bouton) == OK){
                sourisX--;
                sourisY--;
                if( (sourisX >= 0 && sourisX < (LARGEUR2 - 2) ) && ( (sourisY - POSY2) < HAUTEUR2 - 2 && (sourisY - POSY2) >= 0) ){
                    
                    if( is_free(sourisY - POSY2, sourisX) ){
                        placer_element(sourisY - POSY2, sourisX, 1, true);
                    }else{
                        placer_element(sourisY - POSY2, sourisX , 0, true);
                    }
                    wrefresh(fenetre_jeu);
                    
                    wrefresh(fenetre_log);
                }
                

            }
            
        }
        else{
            switch (i)
            {
                case KEY_F(2):
                    quitter = TRUE;
                    break;
                
                case '\n' : case '\t': case '\r': case '\0': break;
                case ' ':
                    wprintw(fenetre_log, "_");
                    wrefresh(fenetre_log);
                    break;
                default:
                    wprintw(fenetre_log, "%c", i);
                    wrefresh(fenetre_log);
                    break;
            }
        }
    }
	
	
	close(fd);

	/*free( matrice ); 
	free( mapBuffer );


	if( ERR == delwin(fenetre_log)){
		perror("Erreur delwin, tentative free...");	
		free(fenetre_log);
	}
	if( ERR == delwin(fenetre_jeu)){
		perror("Erreur delwin, tentative free...");	
		free(fenetre_jeu);
	}
	if( ERR == delwin(fenetre_etat)){
		perror("Erreur delwin, tentative free...");	
		free(fenetre_etat);
	}
	if( ERR == delwin(box_log )){
		perror("Erreur delwin, tentative free...");	
		free(box_log);
	}
	if( ERR == delwin(box_jeu )){
		perror("Erreur delwin, tentative free...");	
		free(box_jeu);
	}
	if( ERR == delwin(box_etat)){
		perror("Erreur delwin, tentative free...");	
		free(box_etat);
	}
*/
	ncurses_stopper();

	exit( EXIT_SUCCESS );

}


/*=== Définitions des fonctions ===*/


int is_free(int y, int x){ return matrice[y][x] == 0; }

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
            
            mvwprintw(fenetre_jeu, y, x, "?");

            
			break;
	}

	if( write == true){
		insertElement(fd, y, x, c);
	}
	
}