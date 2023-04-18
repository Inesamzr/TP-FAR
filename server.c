#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int fin=0;
int dS;
int dSC1;
int dSC2;

//pthread_mutex_t fin_mutex;
//server reçoi un message du client 1 et l'envoi au client 2
void* client1(void* arg) {
  //int dSC1 = *(int*)arg;
  int taille_msg_c1; // socket de communication avec le client 1
  char msg_c1[taille_msg_c1]; // message reçu du client 1
   // taille du message reçu du client 1

  while(1) {
    // réception de la taille du message du client 1
    if (recv(dSC1, &taille_msg_c1, sizeof(taille_msg_c1), 0) <= 0) {
      perror("Erreur de réception du message du client 1");
      break;
    }

    // réception du message du client 1
    if (recv(dSC1, msg_c1, taille_msg_c1, 0) <= 0) {
      perror("Erreur de réception du message du client 1");
      break;
    }

    printf("Message reçu du client 1 : %s\n", msg_c1);

    // envoi du message du client 1 au client 2
    //pthread_mutex_lock(&fin_mutex);
    if (!fin) {
      send(dSC2, &taille_msg_c1, sizeof(taille_msg_c1), 0); // envoi de la taille du message
      send(dSC2, msg_c1, taille_msg_c1, 0); // envoi du message
      printf("Message du client 1 envoyé au client 2 : %s\n", msg_c1);
    }
    //pthread_mutex_unlock(&fin_mutex);

    if (strcmp(msg_c1, "fin\n") == 0) {
     //pthread_mutex_lock(&fin_mutex);
      fin = 1;
      //pthread_mutex_unlock(&fin_mutex);
      break;
    }
  }

  close(dSC1);
  pthread_exit(NULL);
}


//server reçoi un message du client 2 et l'envoi au client 1
void* client2(void* arg) {
  //int dSC2 = *(int*)arg;
  int taille_msg_c2; // socket de communication avec le client 2
  char msg_c2[taille_msg_c2]; // message reçu du client 2
   // taille du message reçu du client 2

  while(1) {
    // réception de la taille du message du client 2
    if (recv(dSC2, &taille_msg_c2, sizeof(taille_msg_c2), 0) <= 0) {
      perror("Erreur de réception du message du client 2");
      break;
    }

    // réception du message du client 2
    if (recv(dSC2, msg_c2, taille_msg_c2, 0) <= 0) {
      perror("Erreur de réception du message du client 2");
      break;
    }

    printf("Message reçu du client 2 : %s\n", msg_c2);

    // envoi du message du client 2 au client 1
    //pthread_mutex_lock(&fin_mutex);
    if (!fin) {
      send(dSC1, &taille_msg_c2, sizeof(taille_msg_c2), 0); // envoi de la taille du message
      send(dSC1, msg_c2, taille_msg_c2, 0); // envoi du message
      printf("Message du client 2 envoyé au client 1 : %s\n", msg_c2);
    }
   // pthread_mutex_unlock(&fin_mutex);

    if (strcmp(msg_c2, "fin\n") == 0) {
      //pthread_mutex_lock(&fin_mutex);
      fin = 1;
      //pthread_mutex_unlock(&fin_mutex);
      break;
    }
  }

  close(dSC2);
  pthread_exit(NULL);
  
}
int main(int argc, char *argv[]) {
  
  printf("Début programme\n");
  //pthread_mutex_init(&fin_mutex, NULL);
  // crée une socket
  dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");

  // paramètre la socket
  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY ;
  ad.sin_port = htons(atoi(argv[1])) ; // la relie à un port
  int bind_res = bind(dS, (struct sockaddr*)&ad, sizeof(ad)); // le bind sert à renommer la socket dans le port que l'on veut pour pouvoir le reutiliser par le client
  if (bind_res == -1) {
    perror("l'erreur est : ");
    exit(1);
  };
  printf("Socket Nommé\n");

  listen(dS, 7) ;
  printf("Mode écoute\n");

// création d'une socket pour le client 1
  struct sockaddr_in aC1 ;
  socklen_t lg1 = sizeof(struct sockaddr_in) ;
  dSC1 = accept(dS, (struct sockaddr*) &aC1,&lg1) ;
  if (dSC1 == -1 ) {
    perror ("l'erreur est : ");
    exit(1);
  };
  printf("Client Connecté\n");

  // création d'une socket pour le client 2
  struct sockaddr_in aC2;
  socklen_t lg2 = sizeof(struct sockaddr_in) ;
  dSC2 = accept(dS, (struct sockaddr*) &aC2,&lg2) ;
  if (dSC2 == -1 ) {
    perror ("l'erreur est : ");
    exit(1);
  };
  printf("Client Connecté\n");

  pthread_t client1_thread;
  pthread_t client2_thread;

  if (pthread_create(&client1_thread, NULL, client1, NULL) != 0) {
    perror("Erreur lors de la création du thread du client 1 : ");
    exit(1);
  }

  if (pthread_create(&client2_thread, NULL, client2, NULL) != 0) {
    perror("Erreur lors de la création du thread du client 2 : ");
    exit(1);
  }

  //pthread_mutex_destroy(&fin_mutex);
  pthread_join(client1_thread, NULL);
  pthread_join(client2_thread, NULL);
 
  printf("Fin du programme");

  int res_shutdown_dSC1 = shutdown(dSC1,2);
  if(res_shutdown_dSC1 == -1){
    perror("Erreur lors de la fermeture de la socket client 1 : ");
  }

  int res_shutdown_dSC2 = shutdown(dSC2, 2);
  if(res_shutdown_dSC2 == -1){
    perror("Erreur lors de la fermeture de la socket client 2 : ");
  }

  int res_shutdown_dS = shutdown(dS, 2);
  if(res_shutdown_dS == -1){
    perror("Erreur lors de la fermeture de la socket serveur : ");
  }
}



