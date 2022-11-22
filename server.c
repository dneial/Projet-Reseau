#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "read_graph.h"
#include "tcp_communication.h"


#define PORT_FILE "server_port.txt"
#define NB_OF_CLIENTS_FILE "clients.txt"

struct Client {
    int socket;
    int in;
    int out;
    struct sockaddr_in addr;
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

int get_nb_of_in_connections(int client_index, struct Graph *graph) {
    int size = graph->sommets;
    int cpt = 0;
    for (int i = 0; i < size; i++) {
        if (graph->matrix[i][client_index]) cpt++;
    }
    return cpt;
}

void distribute_addresses(struct Client *clients, struct Graph *graph){
    struct Client *source;
    struct Client *destination;

    for(int i=0; i<graph->sommets; i++){
        for(int j=0; j<graph->sommets; j++){
            if(graph->matrix[i][j]){
                source = &clients[j];
                destination = &clients[i];
                printf("[+] Server: sending address of %d to %d: %s:%d\n", j+1, i+1,
                       inet_ntoa(source->addr.sin_addr),
                       ntohs(source->addr.sin_port));
                send_tcp(destination->socket, &source->addr, sizeof(struct sockaddr_in));
            }
        }
    }
}



void load_graph(FILE *file, struct Graph *graph){

    read_headers(file, 0);
    read_graph_info(file, graph);

    printf("[+] Server: l%dading graph\n", 0);

    // why god why ?
    int tableau_bizarre_qui_segv_si_pas_utilise[graph->aretes];

    create_matrix(graph);
    read_graph(file, graph);

}

void write_port(int port){
    FILE *f = fopen(PORT_FILE, "w");
    fprintf(f, "%d", port);
    fclose(f);
}

void write_clients(int nb_clients){
    FILE *f = fopen(NB_OF_CLIENTS_FILE, "w");
    fprintf(f, "%d", nb_clients);
    fclose(f);
}

void print_clients_info(struct Client *clients, int size) {
    for (int i = 0; i < size; i++) {
        printf("[+] Server: client %d\n"
               "    Socket #%d\n"
               "    In: %d\n"
               "    Out: %d\n"
               "    Address : %d\n", i, clients[i].socket, clients[i].in, clients[i].out,
               ntohs(clients[i].addr.sin_port));
    }
}


int main(int argc, char *argv[]){
    // paramètre = num port socket d'écoute
    // argv[1] = fichier contenant le graphe
    // argv[2] = option : -v pour afficher les infos des clients
    //                    -n pour afficher l'adresse et le port du serveur à fournir aux clients

    if (argc < 2){
        printf("[-] Utilisation: %s <GRAPH_FILE> [-v|--verbose] [-n|--network <adresse_IP>]\n", argv[0]);
        exit(1);
    }

    int verbose = 0;
    int network = 0;
    char servAddr[INET_ADDRSTRLEN] = "0.0.0.0";

    if (argc > 2){
        if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0){
            printf("[+] Server: verbose mode\n");
            verbose = 1;

            if (argc > 3){
                if (strcmp(argv[3], "-n") == 0 || strcmp(argv[3], "--network") == 0){
                    printf("[+] Server: network mode\n");
                    network = 1;
                    strcpy(servAddr, argv[argc-1]);
                    printf("%s\n", servAddr);
                }
            }
        }

        else if (strcmp(argv[2], "-n") == 0 || strcmp(argv[2], "--network") == 0){
            printf("[+] Server: network mode\n");
            network = 1;
            strcpy(servAddr, argv[argc-1]);

            if (argc > 3){
                if (strcmp(argv[3], "-v") == 0 || strcmp(argv[3], "--verbose") == 0){
                    printf("[+] Server: verbose mode\n");
                    verbose = 1;
                }
            }
        }
        else{
            printf("[-] Server: unknown option %s\n", argv[2]);
            exit(1);
        }
    }

    // Lire le fichier décrivant la configuration du reseau
    // Enregistrer sous la forme de la structure Graph
    const char *FILENAME = argv[1];
    printf("Reading %s...\n", FILENAME);

    FILE *f = fopen(FILENAME, "r");
    if(f == NULL) {
        perror("File not found");
        exit(1);
    }
    struct Graph graph;
    load_graph(f, &graph);
    printf("Graph: %d sommets and %d arêtes\n", graph.sommets, graph.aretes);
    fclose(f);

    //nombre de clients à relier
    const int NB_CLIENTS = graph.sommets;
    struct Client clients[NB_CLIENTS];


    /* creation socket permet recevoir demandes connexion.*/
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

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
        printf("[+] Server: mise en écoute @%s:%d\n", servAddr,
                                                              ntohs(server.sin_port));

    if (!network)
    {
        write_port(ntohs(server.sin_port));
        write_clients(graph.sommets);
    }
    else
    {
        printf("\n[+] Server: veuillez renseignez les informations suivantes au processus client:\n");
        printf("[+] Server: Addresse = %s\n", servAddr);
        printf("[+] Server: Port = %d\n", ntohs(server.sin_port));
        printf("[+] Server: nb clients = %d\n\n", graph.sommets);
    }

    /* attendre et traiter demande connexion client.
       serveur accepte demande = creation nouvelle socket
       connectée au client à utiliser pour
       communiquer avec lui.*/

    //tableau des adresses des sockets clients ici

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
        printf("[+] Server: connexion acceptée from %s:%d\n", inet_ntoa(adC.sin_addr),
                                                              ntohs(adC.sin_port));

        //on stocke la socket pour pouvoir recommuniquer avec le client plus tard
        clients[cptClient].socket = dsCv;

        /* affichage adresse socket client accepté :
           adresse IP et numéro de port de structure adC.
           Attention conversions format réseau -> format hôte.
           fonction inet_ntoa(..) pour l'IP. */


        //Envoyer msg au client indiquant le nombre de sockets d'entrée et de sortie.
        int out = get_nb_of_out_connections(cptClient, &graph);
        int in = get_nb_of_in_connections(cptClient, &graph);

        int in_out[2];
        in_out[0] = in;
        in_out[1] = out;

        clients[cptClient].in = in;
        clients[cptClient].out = out;

        int sent = send_tcp(dsCv, in_out, sizeof(int) * 2);
        if(sent < 0){
            perror("[-] Server: error sending number of voisins");
            exit(1);
        }
        // rcv adresses des sockets d'entrée du client i

        struct sockaddr_in c_in;

        if(in > 0){
            printf("[+] Server: about to receive %d addresses from client %d\n", in, cptClient+1);
            int received = receive_tcp(dsCv, &c_in, sizeof(struct sockaddr_in));
            if(received>0) {
                c_in.sin_addr = adC.sin_addr;
                printf("[+] Server: adresse from %d: %s:%d\n", cptClient+1, inet_ntoa(c_in.sin_addr),ntohs(c_in.sin_port));
                clients[cptClient].addr = c_in;
            }
            else{
                perror("[-] Server: error receiving adresse\n");
                exit(1);
            }
        }

        printf("[+] Server: fin du premier échange avec le client %i \n", cptClient+1);
        printf("-----------------\n");
        cptClient++;
    }

    printf("[+] Server: tous les clients sont prêts\n");
    print_clients_info(clients, NB_CLIENTS);
    distribute_addresses(clients, &graph);

    /*fermeture socket demandes */
    close_sockets(clients, NB_CLIENTS);
    close(server_socket);
    free_matrix(&graph);

    printf("[+] Server: je termine\n");

    return 0;
}
//
// Created by daniel on 10/26/22.
//
