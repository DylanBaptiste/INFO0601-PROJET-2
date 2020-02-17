#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "file_utils.h"

#include "config.h"


/**
 * @brief ouvre un fichier
 * 
 * @param path chemin du fichier
 * @return int file descriptor du fichier ouvert
 */
int openFile(char* path)
{
    int fd = open(path, O_RDWR);
    
    if (fd >= 0){
        return fd;
    }
    else if (errno == ENOENT){
        fd = open(path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
        
        if (fd >= 0 ){
            constructeurFile(fd);
        }
        else{
            perror("Erreur lors de l'ouverture du fichier");
            exit(EXIT_FAILURE);
        }
    }
    else {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int openFileSim(char* simFile, unsigned char *buff)
{
    
    int fd;
    char* path = malloc(sizeof(char) * strlen(simFile) + 1);
    
    if(path == NULL){
        perror("erreur lors du malloc createSim");
        exit(EXIT_FAILURE);
    }

    strcat(path, getFileBase(simFile));
    strcat(path, ".bin");

    fd = open(simFile, O_RDWR);
    
    if (fd >= 0){
        readMap(fd, buff);
    }
    else if (errno == ENOENT){
        fd = open(simFile, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
        
        if (fd >= 0 ){
            fd = createSim(simFile, fd, buff); 
        }
        else{
            perror("Erreur lors de l'ouverture du fichier\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
    perror("Erreur lors de l'ouverture du fichier");
    exit(EXIT_FAILURE);
    }
    return fd;
}

/**
 * @brief ecrit un buffer dans un fichier
 * 
 * @param buffer message à ecrire
 * @param fd file descriptor du fichier à remplire
 */
void writeMap(unsigned char* buffer, int fd)
{
    int i;
    
    for(i = 0; i < (LARGEUR2 - 2)*(HAUTEUR2 - 2); i++){
        if(write(fd, buffer+i, sizeof(unsigned char)) == -1 ){
            perror("Erreur Ecriture");
            exit(EXIT_FAILURE);
        }
    }

   
}
    
/**
 * Construit un decor vide
 */
void constructeurFile(int fd)
{
    int i;
    unsigned char caseVide = 0;


    for(i = 0; i < (LARGEUR2 - 2)*(HAUTEUR2 - 2); i++){
        if(write(fd, &caseVide, sizeof(unsigned char)) == -1 ){
        perror("Erreur Ecriture");
        exit(EXIT_FAILURE);
    }
    }

} 

/**
 * @brief copie un fichier dans un buffer
 * 
 * @param fileName le chemin du fichier a copier
 * @param buff le buffer qui doit recevoir le contenu du fichier
 * @return int 
 */
void readMap(int fd, unsigned char* buff)
{
    int taille = 0;
    int state;
    unsigned char lecture;
    
    if (lseek(fd,0, SEEK_SET) == -1 ){
        perror("Erreur Lors du LSEEK readmap");
        exit(EXIT_FAILURE);
    }

    do{
        state = read(fd, &lecture, sizeof(unsigned char));
        if(state == -1){
            perror("Erreur lors de la lecture de la map, case par defaut");
            buff[taille++] = 0;
        }else{
            buff[taille++] = lecture;
        }
        
    }while(taille <  (LARGEUR2 - 2)*(HAUTEUR2 - 2) );


    if ( lseek(fd, (LARGEUR2 - 2)*(HAUTEUR2 - 2), SEEK_SET) == -1 ){
        perror("Erreur Lors du LSEEK readMap");
        exit(EXIT_FAILURE);
    }
    
    
}

/**
 * @brief créé une simulation à partir d'un decor
 * 
 * @param decor 
 */
int createSim(char* simFile, int sim_d, unsigned char* buff){
    
    
    int decor_d;
    char* path = malloc(sizeof(char) * strlen(simFile) + 1);
    if(path == NULL){
        perror("erreur lors du malloc createSim");
        exit(EXIT_FAILURE);
    }

    strcat(path, getFileBase(simFile));
    strcat(path, ".bin");
    decor_d = openFile(path);

    readMap(decor_d, buff);

    writeMap(buff, sim_d);

    return sim_d;
}

/**
 * @brief get l'extention du fichier
 * 
 * @param path chemin du fichier
 * @return char* l'extention du fichier
 */
char* getFileExt(const char* path){
    char *dot = strrchr(path, '.');
    if(!dot || dot == path) return "";
    return dot + 1;
}

/**
 * @brief get le chemin sans l'extention
 * 
 * @param path le chemin du fichier
 * @return char* le chemin sans l'extention
 */
char* getFileBase (const char* path) {
    
    int i;
    size_t extSize  = strlen(getFileExt(path)) + 1;
    size_t baseSize = strlen(path) - extSize + 1;
    char* base = (char*)malloc(baseSize);
    if(base == NULL){
        perror("erreur lors du malloc createSim");
        exit(EXIT_FAILURE);
    }
    

    for(i = 0; (i < baseSize - 1) && (i < MAXFNAME); ++i){
        base[i] = path[i];
    }
    base[i++] = '\0';

    return base;
}

/**
 * @brief insert une valeur à une position spécifié
 * 
 * @param x 
 * @param y 
 * @param fd file descriptor du fichier a modifier
 */
void insertElement(int fd, int x, int y, unsigned char element){
    if ( lseek(fd, (LARGEUR2 - 2)*x+y, SEEK_SET) == -1 ){
        perror("Erreur Lors du LSEEK insertElement");
        exit(EXIT_FAILURE);
    }
   
    if(write(fd, &element, sizeof(unsigned char)) == -1 ){
        perror("Erreur Ecriture");
        exit(EXIT_FAILURE);
    }
}


