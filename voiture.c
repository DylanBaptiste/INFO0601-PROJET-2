#include <string.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>
#include<signal.h> 
#include "ncurses_utils.h"
#include "file_utils.h"

#include "config.h"

int map[ MAP_HAUTEUR * MAP_LARGEUR ];

void handler(int s){
    printf("signal: %d\n", s);
}

int main(int argc, char** argv) {
    signal(SIGINT, handler); 
    while (1) ; 
    printf("\n%d\n", getpid());

    exit(EXIT_SUCCESS);
		
}
