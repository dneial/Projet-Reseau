#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "read_graph.c"

// Rôle du serveur : accepter la demande de connexion d'un client,
// recevoir une chaîne de caractères, afficher cette chaîne et
// renvoyer au client le nombre d'octets reçus par le serveur.

#define PORT_FILE "server_port.txt"


struct Client {
    int socket;
    int in;
    int out;
    struct sockaddr_in *addr;
};

void close_sockets(struct Client *c, int size)
{
    for (int i = 0; i < size; i++)
    {
        close(c[i].socket);
    }
}

void afficheTab(int *tab,int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("client %i : %d \n",i, tab[i] );
    }
}


int get_nb_of_out_connections(int client_index, struct Graph *graph){
    int size = graph->sommets;
    int cpt = 0;
    for(int i=0; i<size; i++){
        if(graph->matrix[client_index][i]) cpt++;
    }
    return cpt;
}

int get_nb_of_in_connections(int client_index, struct Graph *graph){
    int size = graph->sommets;
    int cpt = 0;
    for(int i=0; i<size; i++){
        if(graph->matrix[i][client_index]) cpt++;
    }
    return cpt;
}

void select_address(struct Client *c, struct sockaddr_in *addr){
    int nb = c->in--;
    for(int i; i<nb; i++){
        addr = &c->addr[i];
        printf("[+] Server: selected %d\n", addr->sin_port );
    }
}


void distribute_addresses(struct Client *clients, struct Graph *graph){
    struct Client sender;
    struct Client receiver;
    struct sockaddr_in addr;

    for(int i=0; i<graph->sommets; i++){
        for(int j=0; j<graph->sommets; j++){
            if(graph->matrix[i][j]){
                sender = clients[i];
                receiver = clients[j];
                printf("[+] Server: sending address of %d to %d: %s:%d\n", sender.socket, receiver.socket,
                                                                                  sender.addr[0].sin_port);
                send(receiver.socket, &addr, sizeof(struct sockaddr_in), 0);
            }
        }
    }
}



void load_graph(FILE *file, struct Graph *graph){
    read_headers(file, 0);
    read_graph_info(file, graph);

    int matrix[graph->aretes];

    create_matrix(graph);
    read_graph(file, graph);
}

void write_port(int port){
    FILE *f = fopen(PORT_FILE, "w");
    fprintf(f, "%d", port);
    fclose(f);
}
void print_clients_info(struct Client *clients, int size){
    for(int i=0; i<size; i++){
        printf("[+] Server: client %d\n"
               "    Socket #%d\n"
               "    In: %d\n"
               "    Out: %d\n", i, clients[i].socket, clients[i].in, clients[i].out);
        for(int j=0; j<clients[i].in; j++){
            printf("    Address #%d: %d\n", j, ntohs(clients[i].addr[j].sin_port));
        }
    }
}


void save_addresses(struct sockaddr_in *addr, int size_in, struct Client *client){
    client->addr = malloc(sizeof(struct sockaddr_in) * size_in);
    for(int i=0; i<size_in; i++){
        client->addr[i] = addr[i];
    }
}

int main(int argc, char *argv[]){

    // paramètre = num port socket d'écoute

    if (argc != 2){
        printf("[-] Utilisation: %s <GRAPH FILE>\n", argv[0]);
        exit(1);
    }

    // Lire le fichier décrivant la configuration du reseau
    // Enregistrer sous la forme de la structure Graph
    const char *FILENAME = argv[1];
    printf("Reading %s...\n", FILENAME);

    FILE *f = fopen(FILENAME, "r");
    struct Graph graph;
    load_graph(f, &graph);
    printf("Graph: %d sommets and %d arêtes\n", graph.sommets, graph.aretes);

    print_matrix(&graph);

    //nombre de clients à relier
    const int NB_CLIENTS = graph.sommets;
    struct Client clients[NB_CLIENTS];


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
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    socklen_t lenServer = sizeof (struct sockaddr_in);

    int binding = bind(server_socket, (struct sockaddr*) &server, sizeof(server));

    if(binding < 0){
        perror("[-] Server: erreur binding");
        close(server_socket);
        exit(1);
    }

    printf("[+] Server: connecté\n");

    /* socket nommée -> ecoute
       dédie socket à réception demandes connexion
       & limiter file demandes connexions */

    int ecoute = listen(server_socket, NB_CLIENTS);

    if (ecoute < 0){
        printf("[-] Server: Erreur mise en écoute\n");
        close(server_socket);
        exit(1);
    }
    if (getsockname(server_socket, (struct sockaddr *)&server, &lenServer) == -1)
        perror("[-] Server: getsockname failed.\n");
    else
        printf("[+] Server: mise en écoute @%s:%d\n", inet_ntoa(server.sin_addr),
                                                              ntohs(server.sin_port));

    write_port(ntohs(server.sin_port));


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
        clients[cptClient].socket = dsCv;

        /* affichage adresse socket client accepté :
           adresse IP et numéro de port de structure adC.
           Attention conversions format réseau -> format hôte.
           fonction inet_ntoa(..) pour l'IP. */


        // char* ipserv = inet_ntoa(adC.sin_addr);
        // int port = htons(server.sin_port);
        // printf("Server: le client %s:%d est connecté  \n", ipserv, port);


        //Envoyer msg au client indiquant le nombre de sockets d'entrée et de sortie.
        int out = get_nb_of_out_connections(cptClient, &graph);
        int in = get_nb_of_in_connections(cptClient, &graph);

        int in_out[2];
        in_out[0] = in;
        in_out[1] = out;

        clients[cptClient].in = in;
        clients[cptClient].out = out;

        int sent = send(dsCv, in_out, sizeof(int) * 2, 0);

        // rcv adresses des sockets d'entrée du client i

        struct sockaddr_in c_in[in];

        if(in > 0){
            printf("[+] Server: about to receive %d addresses from client %d\n", in, cptClient+1);
            int received = recv(dsCv, c_in, sizeof(struct sockaddr_in) * in, 0);
            if(received>0) {
                printf("[+] Server: received %d addresses from %d\n", in, cptClient+1);
                save_addresses(c_in, in, &clients[cptClient]);
            }
            else{
                perror("[-] Server: error receiving addresses\n");
                exit(1);
            }
        }
        for(int i=0; i<in; i++){
            printf("[+} Server: Saved address %s:%d\n", inet_ntoa(clients[cptClient].addr[i].sin_addr),
                   ntohs(clients[cptClient].addr[i].sin_port));
        }
        printf("[+] Server: fin du premier échange avec le client %i \n", cptClient+1);
        cptClient++;
    }

    printf("[+] Server: tous les clients sont prêts\n");
    print_clients_info(clients, NB_CLIENTS);
    distribute_addresses(clients, &graph);
    exit(0);

    /*fermeture socket demandes */
    close_sockets(clients, NB_CLIENTS);
    printf("[+] Server: je termine\n");

    return 0;
}
//
// Created by daniel on 10/26/22.
//