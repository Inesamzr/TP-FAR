#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


int dS;

void *send_message();
void *receive_message();

int main(int argc, char *argv[]) {

  // crée la socket 
  printf("Début programme\n");
  dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1) {
    perror("l'erreur est : ");
    exit(1);
  };
  printf("Socket Créée\n");

    // fait le lien avec le serveur 
  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
  aS.sin_port = htons(atoi(argv[2])) ;
  socklen_t lgA = sizeof(struct sockaddr_in) ;
  int co_res = connect(dS, (struct sockaddr *) &aS, lgA) ;
  if (co_res == -1) {
    perror("l'erreur est : ");
    exit(1);
  };

  printf("Socket Connecté\n");

  pthread_t send_thread;
  pthread_t receive_thread;

  if (pthread_create(&send_thread, NULL, send_message, NULL) != 0) {
    perror("Erreur lors de la création du thread d'envoi de message");
    exit(1);
  }

  if (pthread_create(&receive_thread, NULL, receive_message, NULL) != 0) {
    perror("Erreur lors de la création du thread de réception de message");
    exit(1);
  }

  pthread_join(send_thread, NULL);
  pthread_join(receive_thread, NULL);

  shutdown(dS, 2);
  printf("Fin du programme\n");
  exit(0);

}

void *send_message() {
  int running = 1;
  while(running) {
    // Envoi du message du client
    char msg1[100]; // taille maximale du message à envoyer
    printf("Entrez le message que vous voulez envoyer ('fin' pour arrêter) : ");
    fgets(msg1, sizeof(msg1), stdin);
    int taille = strlen(msg1);
    int send_taille = send(dS, &taille, 4, 0);
    if (send_taille == -1 ) {
      perror ("l'erreur est taille du message: ");
      exit(1);
    };
    int sendi = send(dS, msg1, taille, 0);
    if (sendi == -1 ) {
      perror ("l'erreur est message : ");
      exit(1);
    };
    printf("Message Envoyé \n");

    //dans le cas ou on arrête le programme
    if (strcmp(msg1, "fin\n") == 0) {
    shutdown(dS, 2);
    printf("Fin du programme\n");
    running = 0; // sortir de la boucle
    }
    
  }
  pthread_exit(NULL);
}

void *receive_message() {

  int running = 1;

  while (running) {
     // Réception du message du serveur
    while(1) {
      int taille_msg;
      int res_t = recv(dS, &taille_msg, 4, 0);
      if (res_t == -1) {
        perror("l'erreur est : ");
        exit(1);
      }
      //dans le cas ou le client avec qui on communique arrête le programme
      if (res_t == 0) {
        printf("Connexion fermée par le serveur\n");
        shutdown(dS, 2);
        exit(0);
      }
      //affiche la taille du message
      printf("taille reçu  : %d\n", taille_msg);

      char msg [(taille_msg)] ;
      int res_mess2 = recv(dS, msg, sizeof(msg), 0) ;
      if (res_mess2 == -1) {
        perror("l'erreur est : ");
        exit(1);
      }
      //affiche le message reçu
      printf("Message reçu : %s \n", msg);

      //dans le cas ou on arrête le programme
      if (strcmp(msg, "fin\n") == 0) {
        shutdown(dS, 2);
        printf("Fin du programme\n");
        running = 0; // sortir de la boucle
      }
      printf(" \n Entrez le message que vous voulez envoyer ('fin' pour arrêter) : \n");
     break;
    }
  }
  pthread_exit(NULL);
}
