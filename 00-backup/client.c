#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


void create_out_sockets(int *tab_sockets, int nb_sockets){
/*    int socket;
    for(int i=0; i<nb_sockets; i++){
        socket = socket(AF_INET, SOCK_STREAM, 0)
        if (socket == -1){
            printf("[-] Client : pb creation socket sortie\n");
            exit(1);
        };
        tab_sockets[i] = socket;
    }*/
}

void create_in_sockets(int *tab_sockets, struct sockaddr_in *addresses, int nb_sockets){
    /*int socket;
    struct sockaddr_in addr;

    for(int i=0; i<nb_sockets; i++) {
        socket = socket(AF_INET, SOCK_STREAM, 0)
        if (socket == -1) {
            printf("[-] Client : pb creation socket sortie\n");
            exit(1);
        };
        addr.sin_family = AF_INET;
        addr.s_addr = INADDR_ANY;
        addr.sin_port = 0;
        if (bind(socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in) < 0)) {
            perror("[-] Client: erreur binding");
            close(socket);
            exit(1);
        }
        addresses[i] = addr;
    }*/
}

int main(int argc, char *argv[]) {

  /* parametres : IP, numéro port serveur, numéro port perso)*/

  if (argc != 4){
    printf("[-] Utilisation: %s <ip_serveur> <port_serveur> <socket port>\n", argv[0]);
    exit(0);
  }

  /* Creation socket Pour Server*/


  int server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket == -1){
    printf("[-] Client: problème création socket\n");
    exit(1);
  }
  printf("[+] Client: création de la socket OK\n");
  
  /* contient adresse socket serveur */

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(server_address.sin_addr));
  server_address.sin_port = htons(atoi(argv[2]));
      
  socklen_t lgAdr = sizeof(struct sockaddr_in);



  /* creation socket permet recevoir demandes connexion.*/

  int socket_entree = socket(PF_INET, SOCK_STREAM, 0);

  if (socket_entree == -1){
    printf("[-] Client: error creating socket @ port %s", argv[2]);
    perror("[-] Client: probleme creation socket voisin");
    exit(1);
  }
  
  printf("[+] Client: création de la socket d'entrée OK\n");
 
  
  /* nommage socket
     Elle aura une ou des IP de la machine sur laquelle 
     le programme sera exécuté et le numéro de port passé
     en paramètre
  */

  struct sockaddr_in address_entree;
  address_entree.sin_family = AF_INET;
  address_entree.sin_addr.s_addr = INADDR_ANY;
  address_entree.sin_port = htons(atoi(argv[3]));
  
  int binding = bind(socket_entree,(struct sockaddr*) &address_entree, sizeof(struct sockaddr_in)); 
  if(binding < 0){
    perror("[-] Client: erreur binding");
    close(socket_entree);
    exit(1);
  }

  printf("[+] Client: nommage OK\n");
  printf("[+] Client: ma socket d'entrée se trouve à l'@ %s:%hu\n", 
          inet_ntoa(address_entree.sin_addr), htons(address_entree.sin_port));

  printf("[+] Client: Je tente la connexion avec le serveur @ %s:%hu\n",
        inet_ntoa(server_address.sin_addr), htons(server_address.sin_port));
  
  int conn = connect(server_socket, 
                    (struct sockaddr*) &server_address, 
                    lgAdr);
 
  if (conn < 0){
    perror("[-] Client: problème de connexion avec server");
    close(server_socket);
    exit(1);
  }
  printf("[+] Client: demande de connexion avec server reussie\n");
  

  /*envoi message port*/
  int snd = send(server_socket, &address_entree, sizeof(struct sockaddr_in), 0);

  if (snd < 0){
    perror("[-] Client: probleme d'envoi\n");
    close(server_socket);
    exit(1);
  }
  else if (snd == 0)
    {
      perror("[-] Client: server socket fermée\n");
      close(server_socket);
      exit(1);
    }

  printf("[+] Client: j'ai déposé %d octets\n", snd);

  struct sockaddr_in addresse_sortie;

  int rcv = recv(server_socket, &addresse_sortie, sizeof(struct sockaddr_in), 0);

  if (server_socket < 0){ 
    perror ( "[-] Client: probleme de reception");
    close(server_socket);
    close(socket_entree);
    exit(1);
  }
  else if (server_socket == 0)
    {
      printf("[-] Client: socket server fermée\n");
      close(server_socket);
      close(socket_entree);
      exit(1);
    }

  printf("[+] Client: j'ai recu %d octets \n", rcv);


  //socket demande connexion 
  int socket_sortie = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_sortie == -1){
    printf("[-] Client : pb creation socket sortie\n");
    exit(1);
  }
  printf("[+] Client: creation de la socket sortie OK\n");
  
 
  //CRÉATION SOCKET ENVOI
  int lgAdr2 = sizeof(struct sockaddr_in);

  printf("[+] Client: mon voisin se trouve à %s:%d\n", inet_ntoa(addresse_sortie.sin_addr), ntohs(addresse_sortie.sin_port));

  //MISE EN ECOUTE POUR ENTRÉE
  int ecoute = listen(socket_entree, 1);
  if (ecoute < 0){
    printf("[-] Client: problème mise en écoute\n");
    close(socket_entree);
    exit(1);
  } 

  printf("[+] Client: je suis en écoute\n");


  /*  int connection_voisin =  connect(socket_sortie, 
                            (struct sockaddr *) &addresse_sortie, 
                            lgAdr2);
  */
  
  while(connect(socket_sortie, (struct sockaddr *) &addresse_sortie, lgAdr2) < 0) {
    perror("[-] Client: connexion failed");  
  }

  printf("[+] Client: demande de connexion effectuée avec succès: %s:%d\n", inet_ntoa(addresse_sortie.sin_addr), ntohs(addresse_sortie.sin_port));

	  
  struct sockaddr_in adress_voisin ; // obtenir adresse client accepté
  socklen_t lgV = sizeof (struct sockaddr_in);


  int accept_voisin = accept(socket_entree, 
                            (struct sockaddr *) &adress_voisin, 
                            &lgV);

  if (accept_voisin < 0){
    perror("[-] Client: connexion réfusé");
  }

  printf("[+] Client: accepted connexion from voisin: %s:%d\n", inet_ntoa(adress_voisin.sin_addr), ntohs(adress_voisin.sin_port));

  char hello[5] = "Hello";
  exit(0);
  
  int say_hello = send(socket_sortie, hello, sizeof(hello), 0);
    
  if (say_hello < 0){
    perror("[-] Client: problem sending message");
    exit(1);
  }

  char *msg;

  int rcv_hello = recv(accept_voisin, &msg, sizeof(hello), 0);

  if (rcv_hello < 0) {
    perror("[-] Client: problem receiving message ");
    exit(1);
  }

  printf("[+] Client: received %d bytes from voisin", rcv_hello);
  printf("[+] Client: message reveiced is %s", msg);
  //fermeture socket server à la fin
	
  close (server_socket);
  close (socket_entree);
  printf("[+] Client: je termine\n");
}
