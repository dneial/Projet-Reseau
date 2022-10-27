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
        printf("[+] Client: je me connecte à %d\n", i+1);
        while (connect(tab_sockets[i], (struct sockaddr *) &addresses[i], sizeof(struct sockaddr_in)) < 0){
            perror("[-] Client: erreur connect. Retrying...\n");
        }
        printf("[+] Client: connecté à %s:%d\n", inet_ntoa(addresses[i].sin_addr),
                ntohs(addresses[i].sin_port));
    }
}

void accept_connections(int *tab_sockets, int *com_sockets, int nb_sockets){
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);
    for(int i=0; i<nb_sockets; i++){
        int dsCv = accept(tab_sockets[i],(struct sockaddr *) &adC, &lgC);
        if (dsCv < 0){
            perror ( "[-] Client: probleme accept");
            close(tab_sockets[i]);
            exit(1);
        }
        com_sockets[i] = dsCv;
        printf("[+] Client: accepté connexion de %s:%d\n", inet_ntoa(adC.sin_addr),
               ntohs(adC.sin_port));
    }
}

int main(int argc, char *argv[]) {

    if (argc != 1){
        printf("[-] Utilisation: %s \n", argv[0]);
        exit(0);
    }

    /* Creation socket Pour Server*/
    FILE *f = fopen(PORT_FILE, "r");

    char *line = malloc(5 * sizeof(char));
    size_t n;

    getline(&line, &n, f);
    fclose(f);

    int server_port = atoi(line);
    free(line);
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

    if (server_socket < 0){
        perror ( "[-] Client: probleme de reception");
        close(server_socket);
        exit(1);
    }
    else if (server_socket == 0)
    {
        printf("[-] Client: socket server fermée\n");
        close(server_socket);
        exit(1);
    }

    printf("NB de in_sockets: %d\nNB de out_sockets: %d\n", in_out[0], in_out[1]);

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
    int rcv_add;
    for(int i=0; i<out; i++){
        rcv_add = recv(server_socket, &out_addresses[i], sizeof(struct sockaddr_in), 0);
    }
    for(int i=0; i<out; i++){
        printf("out_addresses[%d]: %s:%d\n", i, inet_ntoa(out_addresses[i].sin_addr),
               ntohs(out_addresses[i].sin_port));
    }
    exit(0);

    establish_connections(out_sockets, out_addresses, out);

    int communication_sockets[in];
    accept_connections(in_sockets, communication_sockets, in);

/*
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


    *//*  int connection_voisin =  connect(socket_sortie,
                              (struct sockaddr *) &addresse_sortie,
                              lgAdr2);
    *//*

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
    printf("[+] Client: je termine\n");*/
}
