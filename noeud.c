//
// Created by daniel on 10/26/22.
//
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT_FILE "server_port.txt"


void create_out_sockets(int *tab_sockets, int nb_sockets){
    int out_socket;
    for(int i=0; i<nb_sockets; i++){
        out_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (out_socket == -1){
            printf("[-] Client : pb creation socket sortie\n");
            exit(1);
        };
        tab_sockets[i] = out_socket;
    }
}

void mise_en_ecoute(int *tab_sockets, struct sockaddr_in *addresses, int nb_sockets){
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    for(int i=0; i<nb_sockets; i++){
        if (listen(tab_sockets[i], 1) < 0){
            perror("[-] Client: erreur listen");
            close(tab_sockets[i]);
            exit(1);
        }
        if (getsockname(tab_sockets[i], (struct sockaddr *) &addr, &len) == -1)
            perror("[-] Client: getsockname failed.\n");
        else{
            printf("[+] Client: socket %d @ port %d\n", i+1, ntohs(addr.sin_port));
            addresses[i] = addr;
        }
    }
}

void create_in_sockets(int *tab_sockets, struct sockaddr_in *addresses, int nb_sockets){
    int in_socket;
    struct sockaddr_in addr;

    for(int i=0; i<nb_sockets; i++) {
        in_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (in_socket == -1) {
            printf("[-] Client : pb creation socket sortie\n");
            exit(1);
        };
        tab_sockets[i] = in_socket;

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = 0;

        if (bind(in_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
            perror("[-] Client: erreur binding");
            close(in_socket);
            exit(1);
        }
    }
    printf("[+] Client : sockets d'entrée créées\n");
    mise_en_ecoute(tab_sockets, addresses, nb_sockets);
}



void establish_connections(int *tab_sockets, struct sockaddr_in *addresses, int nb_sockets){
    for(int i=0; i<nb_sockets; i++){
        printf("[+] Noeud: je me connecte à %d\n", i+1);
        while (connect(tab_sockets[i], (struct sockaddr *) &addresses[i], sizeof(struct sockaddr_in)) < 0){
            perror("[-] Noeud: erreur connect. Retrying...\n");
        }
        printf("[+] Noeud: connecté à %s:%d\n", inet_ntoa(addresses[i].sin_addr),
                ntohs(addresses[i].sin_port));
    }
}

void accept_connections(int *tab_sockets, int *com_sockets, int nb_sockets){
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);
    for(int i=0; i<nb_sockets; i++){
        int dsCv = accept(tab_sockets[i],(struct sockaddr *) &adC, &lgC);
        if (dsCv < 0){
            perror ( "[-] Noeud: probleme accept");
            close(tab_sockets[i]);
            exit(1);
        }
        com_sockets[i] = dsCv;
        printf("[+] Noeud: accepté connexion de %d @ %s:%d\n", dsCv, inet_ntoa(adC.sin_addr),
               ntohs(adC.sin_port));
    }
}

void send_msg(int *tab_sockets, struct sockaddr_in *addr, int nb_sockets, char *msg, size_t msg_size){
    if(nb_sockets > 0){
        printf("[+] Noeud: msg à envoyer : %s\n", msg);
        printf("[+] Noeud: taille du msg à envoyer : %ld\n", msg_size);
    }

    for(int i=0; i<nb_sockets; i++){
        while (send(tab_sockets[i], msg, msg_size, 0) < 0){
            perror("[-] Noeud: problem sending message.");
            printf("[-] Noeud: retrying...\n");
        }
        printf("[+] Noeud: msg sent to: %s:%d\n", inet_ntoa(addr[i].sin_addr),
                                                   ntohs(addr[i].sin_port));
    }
}

void receive_msg(int socket_descriptor, size_t msg_size){
    char *msg = malloc(msg_size);
    int rcv = recv(socket_descriptor, msg, msg_size, 0);
    if(rcv < 0){
        perror("[-] Noeud: problem receiving message");
        exit(1);
    }
    printf("[+] Noeud: received msg from neighbour: %s\n", msg);
    free(msg);
}



int read_server_port(){
    FILE *f = fopen(PORT_FILE, "r");
    if(f == NULL) {
        printf("File not found: %s\n", PORT_FILE);
        exit(1);
    }
    char *line = malloc(sizeof(char)*5);
    size_t n;
    getline(&line, &n, f);
    fclose(f);
    int port = atoi(line);
    free(line);
    return port;
}


void close_sockets(int *sockets, int size)
{
    for (int i = 0; i < size; i++)
    {
        close(sockets[i]);
    }
}


int main(int argc, char *argv[]) {

    if (argc != 1){
        printf("[-] Utilisation: %s \n", argv[0]);
        exit(0);
    }

    /* Creation socket Pour Server*/
    int server_port = read_server_port();
    printf("found port: %d\n", server_port);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1){
        printf("[-] Client: problème création socket\n");
        exit(1);
    }
    printf("[+] Client: création de la socket OK\n");

    /* contient adresse socket serveur */

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &(server_address.sin_addr));
    server_address.sin_port = htons(server_port);

    socklen_t lgAdr = sizeof(struct sockaddr_in);


    printf("[+] Client: Je tente la connexion avec le serveur @ %s:%d\n",
           inet_ntoa(server_address.sin_addr), server_port);

    int conn = connect(server_socket,
                       (struct sockaddr*) &server_address,
                       lgAdr);
    if (conn < 0){
        perror("[-] Client: problème de connexion avec server");
        close(server_socket);
        exit(1);
    }
    printf("[+] Client: demande de connexion avec server reussie\n");

    int in_out[2];

    int rcv = recv(server_socket, in_out, sizeof(int)*2, 0);

    if (rcv < 0){
        perror ( "[-] Client: probleme de reception");
        close(server_socket);
        exit(1);
    }
    else if (rcv == 0)
    {
        printf("[-] Client: socket server fermée\n");
        close(server_socket);
        exit(1);
    }

    int in = in_out[0];
    int out = in_out[1];

    int in_sockets[in];
    struct sockaddr_in in_addresses[in];
    create_in_sockets(in_sockets, in_addresses, in);
    printf("[+] Client: creation des sockets in OK\n");



    int out_sockets[out];
    struct sockaddr_in out_addresses[out];
    create_out_sockets(out_sockets, out);
    printf("[+] Client: creation des sockets out OK\n");


    int send_in = send(server_socket, in_addresses, sizeof(struct sockaddr_in) * in, 0);
    if (send_in < 0){
        perror("[-] Client: probleme d'envoi des adresses in");
        close(server_socket);
        exit(1);
    }
    printf("[+] Client: envoi des adresses_in OK\n");

    /* receive out addresses from server and assign them to out sockets */

    struct sockaddr_in out_add;
    for(int i=0; i<out; i++){
        int rcv = recv(server_socket, &out_add, sizeof(struct sockaddr_in), 0);
        if (rcv < 0){
            perror ( "[-] Client: probleme de reception");
            close(server_socket);
            exit(1);
        }
        else if (rcv == 0)
        {
            printf("[-] Client: socket server fermée\n");
            close(server_socket);
            exit(1);
        }
        out_addresses[i] = out_add;
    }
    printf("[+] Client: reception des adresses out OK\n");

    for(int i=0; i<out; i++){
        printf("out_addresses[%d]: %s:%d\n", i, inet_ntoa(out_addresses[i].sin_addr),
               ntohs(out_addresses[i].sin_port));
    }

    establish_connections(out_sockets, out_addresses, out);

    int communication_sockets[in];
    accept_connections(in_sockets, communication_sockets, in);


    char msg[5] = "hello";

    send_msg(out_sockets, out_addresses, out, msg, sizeof(msg));


    for(int i=0; i<in; i++){
        receive_msg(communication_sockets[i], sizeof(msg));
    }


    close_sockets(in_sockets, in);
    close_sockets(out_sockets, out);
    close_sockets(communication_sockets, in);

    close(server_socket);

    printf("[+] Noeud: je termine\n");

}