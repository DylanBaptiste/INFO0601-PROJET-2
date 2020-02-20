#include "FileMessage.h"

int main() {
  int msqid;

  /* Récupération de la file */
  if((msqid = msgget((key_t)CLE, 0)) == -1) {
    if(errno == ENOENT)
      fprintf(stderr, "Aucune file présente avec la clé %d.\n", (key_t)CLE);
    else
      perror("Erreur lors de la récupération de la file ");
    exit(EXIT_FAILURE);
  }

  /* Suppression de la file */
  if(msgctl(msqid, IPC_RMID, 0) == -1) {
    perror("Erreur lors de la suppression de la file ");
    exit(EXIT_FAILURE);
  }
  printf("File supprimée.\n");

  return EXIT_SUCCESS;
}