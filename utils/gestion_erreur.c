#include "config.h"
#include "gestion_erreur.h"

/*gerer_erreur(!file, "Le fichier n'existe pas", __FILE__, __LINE__);*/

void gerer_erreur(int cond, const char* custom_message, const char* file, const unsigned long line) {
    if (cond) {
        if(DEV) fprintf(stderr, "%s: %s\tat %s:%ld\n", custom_message, strerror(errno), file, line);
        else    fprintf(stderr, "%s", custom_message);
        
        exit(EXIT_FAILURE);
    }
}
