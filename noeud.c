//
// Created by daniel on 10/26/22.
//
//#include <stdio.h>
//#include <sys/socket.h>
//#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "tcp_communication.c"

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

int mise_en_ecoute(int socket, struct sockaddr_in *addresse, int in){

    socklen_t len = sizeof(struct sockaddr_in);

    if (listen(socket, in) < 0){
        perror("[-] Client: erreur listen");
        close(socket);
        exit(1);
    }
    if (getsockname(socket, (struct sockaddr *) addresse, &len) == -1)
        perror("[-] Client: getsockname failed.\n");
    else{
        printf("[+] Client: socket %d @ ip:port %s:%d\n", socket, inet_ntoa(addresse->sin_addr),ntohs(addresse->sin_port));
    }

    return 0;
}

int create_in_socket(struct sockaddr_in *addresse, int in){
    int in_socket;
//    struct sockaddr_in addr;

    in_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (in_socket == -1) {
        printf("[-] Client : pb creation socket sortie\n");
        exit(1);
    };

    addresse->sin_family = AF_INET;
    addresse->sin_addr.s_addr = INADDR_ANY;
    addresse->sin_port = 0;

    if (bind(in_socket, (struct sockaddr *) addresse, sizeof(struct sockaddr_in)) < 0) {
        perror("[-] Client: erreur binding");
        close(in_socket);
        exit(1);
    }

    printf("[+] Client: creation de la socket d'écoute in OK\n");
    mise_en_ecoute(in_socket, addresse, in);
    return in_socket;
}

int establish_connections(int *tab_sockets, struct sockaddr_in *addresses, int nb_sockets){
    int connections = 0;
    for(int i=0; i<nb_sockets; i++){
        printf("[+] Noeud: je me connecte à %d\n", i+1);
        if (connect(tab_sockets[i], (struct sockaddr *) &addresses[i], sizeof(struct sockaddr_in)) < 0) {
            perror("[-] Noeud: erreur connect. Retrying...\n");
            exit(1);
        }
        connections++;
    }
    return connections;
}

int accept_connections(int socket, int *com_sockets, int nb_sockets){
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);
    int accepted = 0;
    for(int i=0; i<nb_sockets; i++){
        int dsCv = accept(socket,(struct sockaddr *) &adC, &lgC);
        if (dsCv < 0){
            perror ( "[-] Noeud: probleme accept");
            close(socket);
            exit(1);
        }
        else {
            com_sockets[i] = dsCv;
            accepted++;
        }
    }

    return accepted;
}

int send_msg(int *tab_sockets, int nb_sockets, char *msg, size_t msg_size){
    if(nb_sockets > 0){
        printf("[+] Noeud: msg à envoyer : %s\n", msg);
        printf("[+] Noeud: taille du msg à envoyer : %ld\n", msg_size);
    }
    int sent = 0;
    for(int i=0; i<nb_sockets; i++){
        int s = send_tcp(tab_sockets[i], msg, msg_size);
        if(s < 0){
            perror("[-] Noeud: problem sending message.");
        }
        else sent++;
    }
    return sent;
}

int receive_msg(int *tab_sockets, int nb_sockets, size_t msg_size){
    char *msg = malloc(msg_size);
    int received = 0;

    for(int i =0; i < nb_sockets; i++){
        int rcv = receive_tcp(tab_sockets[i], msg, msg_size);
        if(rcv < 0){
            perror("[-] Noeud: problem receiving message");
            exit(1);
        } else received++;
    }
    free(msg);

    return received;
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

    if (argc > 5){
        printf("[-] Utilisation: %s [-v|--verbose] [-n|--network <IP> <PORT>]\n", argv[0]);
        exit(0);
    }

    int verbose = 0;
    int network = 0;
    char server_ip[16];
    int server_port;

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0){
            printf("[+] Client: verbose mode\n");
            verbose = 1;

            if (argc > 2){
                if (strcmp(argv[2], "-n") == 0 || strcmp(argv[2], "--network") == 0){
                    printf("[+] Client: Network mode\n");
                    if (argc == 5){
                        network = 1;
                        //tester si format valide normalement
                        strcpy(server_ip, argv[3]);
                        server_port = atoi(argv[4]);
                    }
                    else{
                        printf("[-] Client: missing arguments for network mode\n");
                        printf("[-] Utilisation: %s [-v|--verbose] [-n|--network <IP> <PORT>]\n", argv[0]);
                        exit(1);
                    }
                }
            }
        }

        else if (strcmp(argv[1], "-n") == 0 || strcmp(argv[1], "--network") == 0){
            printf("[+] Client: network mode\n");
            if (argc > 3){
                network = 1;
                //tester si format valide normalement
                strcpy(server_ip, argv[2]);
                server_port = atoi(argv[3]);
            }
            else{
                printf("[-] Client: missing arguments for network mode\n");
                printf("[-] Utilisation: %s [-v|--verbose] [-n|--network <IP> <PORT>]\n", argv[0]);
                exit(1);
            }
            network = 1;

            if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0){
                printf("[+] Client: verbose mode\n");
                verbose = 1;
            }
        }
        else{
            printf("[-] Client: unknown option %s\n", argv[1]);
            exit(1);
        }
    }

    /* Creation socket Pour Server*/

    if (!network)
    {
        strcpy(server_ip, "0.0.0.0");
        server_port = read_server_port();
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1){
        printf("[-] Client: problème création socket\n");
        exit(1);
    }
    printf("[+] Client: création de la socket OK\n");

    /* contient adresse socket serveur */

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(server_address.sin_addr));
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

    int rcv = receive_tcp(server_socket, in_out, sizeof(int)*2);

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
    printf("[+] Client: %d connexions entrantes et %d connexions sortantes\n", in_out[0], in_out[1]);

    int in = in_out[0];
    int out = in_out[1];

    struct sockaddr_in in_addresse;
    int in_socket = -1;
    if(in > 0) in_socket = create_in_socket(&in_addresse, in);

    int out_sockets[out];
    struct sockaddr_in out_addresses[out];
    create_out_sockets(out_sockets, out);
    printf("[+] Client: creation des sockets out OK\n");

    if(in > 0){
        int send_in = send_tcp(server_socket, &in_addresse, sizeof(struct sockaddr_in));
        if (send_in < 0){
            perror("[-] Client: probleme d'envoi des adresses in");
            close(server_socket);
            exit(1);
        }
        printf("[+] Client: envoi des adresses_in OK\n");
    }

    /* receive out addresses from server and assign them to out sockets */

    struct sockaddr_in out_add;
    for(int i=0; i<out; i++){
        int rcv = receive_tcp(server_socket, &out_add, sizeof(struct sockaddr_in));
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

    int connections = establish_connections(out_sockets, out_addresses, out);
    printf("[+] Client: %d connexions sortantes établies\n", connections);

    int communication_sockets[in];
    int accepted = accept_connections(in_socket, communication_sockets, in);
    printf("[+] Client: %d connexions entrantes acceptées\n", accepted);


    char msg[5] = "hello";

    int envoyes = send_msg(out_sockets,out, msg, sizeof(msg));
    printf("[+] Noeud: j'ai envoyé %d messages (sur %d voisins sortants)\n", envoyes, out);

    int recus = receive_msg(communication_sockets, in, sizeof(msg));
    printf("[+] Noeud: j'ai reçu %d messages (sur %d voisins entrants)\n", recus, in);



    printf("[+] Noeud: je termine\n");

    sleep(180);

    close_sockets(out_sockets, out);
    close_sockets(communication_sockets, in);

    close(server_socket);
    close(in_socket);


}
