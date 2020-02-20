#include "FileMessage.h"
#include <signal.h>

int noStopSignal = 1;

void handlerSignal(int sig){
 
  printf("SIGINT RECU ARRET DEMANDE  %d\n ",sig);
  if (sig == SIGINT){
    noStopSignal = 0;   
  }
}


int CreationFile(){
  int msqid;
  if((msqid = msgget((key_t)CLE, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
    if(errno == EEXIST)
      fprintf(stderr, "Erreur : file (cle=%d) existante\n", CLE);
    else
      perror("Erreur lors de la creation de la file ");
    exit(EXIT_FAILURE);
  }
  return msqid;
}

void ReceptionFileConfig(msqid){
  requete_t requete;
  pid_t pid;
  /* IPC_NOWAIT
  *  Revient immédiatement si aucun message du type désiré n'est présent. L'appel système échoue et errno est renseignée avec ENOMSG. 
  */
  if(msgrcv(msqid, &requete, sizeof(requete_t) - sizeof(long), TYPE_CONFIG | TYPE_MODIFCARTE, IPC_NOWAIT ) == -1) {

    if (errno != ENOMSG){
    perror("Erreur lors de la réception d'une requête ");
    exit(EXIT_FAILURE);}
  }
  else{
    printf("coucou");
    if (requete.type == TYPE_CONFIG){
      pid = requete.data.RecupConfig.pid;
      printf("Serveur : requête reçue son pid est : %d\n", requete.data.RecupConfig.pid);
      ReponseFile(msqid,pid);
      printf("Serveur : reponse send");
    }
    else if(requete.type == TYPE_MODIFCARTE){
      /*LOWL JE SAIS PAS IL FAUT FAIRE QUOI LA */
      printf("Type modif recu ");
    }
  }
}

void ReponseFile(msqid,pid){

  requete_t requete;
  requete.type = pid;
  requete.data.SendConfig.cle_sem = 10;
  requete.data.SendConfig.cle_smp = 10;

  if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
    perror("Erreur lors de l'envoi de la reponse ");
    exit(EXIT_FAILURE);
  }
  printf("Serveur : réponse envoyée.\n");
}

void SupprimerFile(msqid){
  
  /* Recuperation de la file */
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
}
int main(int argc, char *argv[]){
  int msqid;
  struct sigaction action;
  printf("Mon pid %d",getpid());
  /* 
  if ((fd = open(argv[1], O_RDWR, S_IRWXU)) == -1)
    {
      perror("Erreur lors de l'ouverture du fichier : ");
      exit(EXIT_FAILURE);
    }
  Signal pour l'arret */
  printf("no stop signal %d \n" ,noStopSignal);
  /* adresse du handler */
  action.sa_handler = &handlerSignal;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT,&action,NULL);


  msqid = CreationFile();
  while (noStopSignal == 1){
  
    ReceptionFileConfig(msqid);
    
  }
  if(noStopSignal == 0){
  printf("Suppression de notre file");
  SupprimerFile(msqid);
  }
  return 0;
}