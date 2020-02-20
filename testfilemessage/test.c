#include "FileMessage.h"


int main() {
  int msqid;int pid;
  requete_t requete;
  

  /* Récupération de la file */
  if((msqid = msgget((key_t)CLE, 0)) == -1) {
    perror("Erreur lors de la récupération de la file ");
    exit(EXIT_FAILURE);
  }
  pid = getpid();
  /* Envoi d'une requête */
  requete.type = TYPE_CONFIG;
  requete.data.RecupConfig.pid = pid;

  if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
    perror("Erreur lors de l'envoi de la requête ");
    exit(EXIT_FAILURE);
  }
  printf("Client : envoi d'une requête.\n");
  printf("%d ",pid);
  /* Réception de la réponse */
  printf("Client : attente de la reponse...\n");
  if(msgrcv(msqid, &requete, sizeof(requete) - sizeof(long), getpid(), 0) == -1) {
    perror("Erreur lors de la réception de la réponse ");
    exit(EXIT_FAILURE);    
  }

  printf("Client : le resultat est %ld \n", requete.type);
  printf("Cle Semaphore %d",requete.data.SendConfig.cle_sem);
  printf("Cle Memoire partage %d",requete.data.SendConfig.cle_smp);
  return EXIT_SUCCESS;
}