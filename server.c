#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "read_graph.c"
//#include "test/strtok.c"

// Rôle du serveur : accepter la demande de connexion d'un client,
// recevoir une chaîne de caractères, afficher cette chaîne et
// renvoyer au client le nombre d'octets reçus par le serveur.

void close_sockets(int *tab, int size)
{
  for (int i = 0; i < size; i++)
    {
      close(tab[i]);
    }	
}

void afficheTab(int *tab,int size)
{
  for (int i = 0; i < size; i++)
    {
      printf("client %i : %d \n",i, tab[i] );
    }
}

void distribute_addresses(int *sockets, struct sockaddr_in *addresses, int size){
  int j;
  for(int i=0; i<size; i++){
    j = (i + 1) % size;
    char *ip = inet_ntoa(addresses[j].sin_addr);
    int port = htons(addresses[j].sin_port);
    printf("Server: j'envoi à %d, l'@ %s:%d\n", i+1, ip, port);
    send(sockets[i], addresses + j, sizeof(struct sockaddr_in), 0);
  }
}

void distribute_addresses2(int *sockets, struct sockaddr_in *addresses, struct Graph *graph){
    printf("je suis à d2\n");
  for(int i=0; i<graph->aretes; i++){
    int s1 = graph->e[i].v1 - 1;
    int s2 = graph->e[i].v2 - 1;
    char *ip = inet_ntoa(addresses[s2].sin_addr);
    int port = htons(addresses[s2].sin_port);
    printf("Server: j'envoi à %d, l'@ %s:%d\n", s1+1, ip, port);
    send(sockets[s1], addresses + s2, sizeof(struct sockaddr_in), 0);
  }
}

int main(int argc, char *argv[]){
  
  // paramètre = num port socket d'écoute
  
  if (argc != 3){
    printf("[-] Utilisation: %s <PORT> <GRAPH FILE>\n", argv[0]);
    exit(1);
  }


    const char *FILENAME = argv[2];
    printf("Reading %s...\n", FILENAME);

    FILE *f = fopen(FILENAME, "r");
    struct Graph graph;

    read_headers(f, 0);
    read_graph_info(f, &graph);


    struct Edge aretes[graph.aretes];
    graph.e = aretes;

    read_graph(f, (struct Graph *) &graph);
    printf("Graph: %d sommets and %d aretes\n", graph.sommets, graph.aretes);


    //nombre de clients à relier
    const int NB_CLIENTS = graph.sommets;
    int tab_sockets[NB_CLIENTS];

  /* creation socket permet recevoir demandes connexion.*/
  int server_socket = socket(PF_INET, SOCK_STREAM, 0);

  if (server_socket == -1){
    perror("[-] Server: probleme creation socket");
    exit(1);
  }
  
  printf("[+] Server: création de la socket : ok\n");
 
  
  /* nommage socket
     Elle aura une ou des IP de la machine sur laquelle 
     le programme sera exécuté et le numéro de port passé
     en paramètre
  */

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(atoi(argv[1]));

  int binding = bind(server_socket, (struct sockaddr*) &server, sizeof(server));

  if(binding < 0){
    perror("[-] Server: erreur binding");
    close(server_socket);
    exit(1);
  }

  printf("[+] Server: connecté @ %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

  /* socket nommée -> ecoute
     dédie socket à réception demandes connexion
     & limiter file demandes connexions */
  
  int ecoute = listen(server_socket, NB_CLIENTS);

  if (ecoute < 0){
    printf("[-] Server: Erreur mise en écoute\n");
    close(server_socket);
    exit(1);
  } 
 
  printf("[+] Server: mise en écoute : ok\n");

  /* attendre et traiter demande connexion client. 
     serveur accepte demande = creation nouvelle socket
     connectée au client à utiliser pour
     communiquer avec lui.*/

  //tableau des adresses des sockets clients ici
  struct sockaddr_in client_sockets[NB_CLIENTS];

  int cptClient = 0;

  while(cptClient < NB_CLIENTS){

    printf("[+] Server: j'attends la demande d'un client\n");

	  
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);

    int dsCv = accept(server_socket,(struct sockaddr *) &adC, &lgC);
    if (dsCv < 0){
      perror ( "[-] Server: probleme accept");
      close(server_socket);
      exit(1);
    }

    //on stocke la socket pour pouvoir recommuniquer avec le client plus tard
    tab_sockets[cptClient] = dsCv;
	  
    /* affichage adresse socket client accepté :
       adresse IP et numéro de port de structure adC. 
       Attention conversions format réseau -> format hôte.
       fonction inet_ntoa(..) pour l'IP. */


    // char* ipserv = inet_ntoa(adC.sin_addr);
    // int port = htons(server.sin_port);
    // printf("Server: le client %s:%d est connecté  \n", ipserv, port);

	  
    //Recevoir msg du client avec l'adresse de socket d'entrée

    /* réception message */
	 
    struct sockaddr_in addr_client;

    int rcv = recv(dsCv, &addr_client, sizeof(struct sockaddr),0);

    if (dsCv < 0){ 
      perror ( "[-] Server: probleme reception\n");
      close(dsCv);
      close(server_socket);
      exit(1);
    }
    else if (dsCv == 0) {
      printf("[-] Server: socket fermée  lors de la récéption du message\n");
      close(dsCv);
      close(server_socket);
      exit(1);
    }
	  
    printf("[+] Server: j'ai recu %d octets \n", rcv);
    printf("[+] Server: received adress from client %d: %s:%d\n", cptClient, inet_ntoa(addr_client.sin_addr), 
	   ntohs(addr_client.sin_port));
    client_sockets[cptClient] = addr_client;

    //incrémentation nb clients enregistrés
    cptClient++;

    printf("[+] Server: fin du premier échange avec le client %i \n", cptClient);
  }

  printf("[+] Server: tous les clients sont prêts\n");


  for(int i=0; i<NB_CLIENTS; i++){
    char *ip = inet_ntoa(client_sockets[i].sin_addr);
    int port = ntohs(client_sockets[i].sin_port);
    printf("Client #%d socket: %s:%d\n", i+1, ip, port);
  }

//  distribute_addresses(tab_sockets, client_sockets, NB_CLIENTS);
  distribute_addresses2(tab_sockets, client_sockets, &graph);
  close_sockets(tab_sockets, NB_CLIENTS);

  
  /*fermeture socket demandes */
  close(server_socket);
  close_sockets(tab_sockets, NB_CLIENTS);
  printf("[+] Server: je termine\n");
}
