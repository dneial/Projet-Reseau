#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "tcp_communication.h"

#define PORT_FILE "server_port.txt"
int INDICE = 0;
int GRAPH_SIZE = 0;
int GRAPH_TYPE = 0;
int SERVER_SOCKET;
int IN_SOCKET;



void close_sockets(int *sockets, int size)
{
    for (int i = 0; i < size; i++)
    {
        close(sockets[i]);
    }
}

void noeud_exit(int *tab_sockets, int nb_sockets){
    close_sockets(tab_sockets, nb_sockets);
    close(SERVER_SOCKET);
    close(IN_SOCKET);
    exit(0);
}

void create_out_sockets(int *tab_sockets, int nb_sockets){
    int out_socket;
    for(int i=0; i<nb_sockets; i++){
        out_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (out_socket == -1){
            printf("[-] Client : pb creation socket sortie\n");
            noeud_exit(tab_sockets, i);
        };
        tab_sockets[i] = out_socket;
    }
}

int mise_en_ecoute(struct sockaddr_in *addresse, int in){

    socklen_t len = sizeof(struct sockaddr_in);

    if (listen(IN_SOCKET, in) < 0){
        perror("[-] Client: erreur listen");
        close(SERVER_SOCKET);
        close(IN_SOCKET);
        exit(1);
    }
    if (getsockname(IN_SOCKET, (struct sockaddr *) addresse, &len) == -1)
        perror("[-] Client: getsockname failed.\n");
    else{
        printf("\n[+] Client : socket @%s:%d\n\n", inet_ntoa(addresse->sin_addr),ntohs(addresse->sin_port));
    }

    return 0;
}

void create_in_socket(struct sockaddr_in *addresse){
//    struct sockaddr_in addr;

    IN_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
    if (IN_SOCKET == -1) {
        printf("[-] Client : pb creation socket sortie\n");
        close(SERVER_SOCKET);
        exit(1);
    };

    addresse->sin_family = AF_INET;
    addresse->sin_addr.s_addr = INADDR_ANY;
    addresse->sin_port = 0;

    if (bind(IN_SOCKET, (struct sockaddr *) addresse, sizeof(struct sockaddr_in)) < 0) {
        perror("[-] Client: erreur binding");
        close(SERVER_SOCKET);
        close(IN_SOCKET);
        exit(1);
    }

    printf("[+] Client: creation de la socket d'écoute in OK\n");
}

int establish_connections(int *tab_sockets, struct Noeud *noeuds, int nb_sockets){
    int connections = 0;
    for(int i=0; i<nb_sockets; i++){
        printf("\n[+] Noeud %d: je me connecte au noeud %d @ %d\n", INDICE, noeuds[i].index, ntohs(noeuds[i].addr.sin_port));
        if (connect(tab_sockets[i], (struct sockaddr *) &noeuds[i].addr, sizeof(struct sockaddr_in)) < 0) {
            perror("[-] Noeud : erreur connect.\n");
            noeud_exit(tab_sockets, i);
        }
        connections++;
    }
    printf("\n");
    return connections;
}

int accept_connections(int *com_sockets, int nb_sockets){
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);
    int accepted = 0;
    for(int i=0; i<nb_sockets; i++){
        int dsCv = accept(IN_SOCKET,(struct sockaddr *) &adC, &lgC);
        if (dsCv < 0){
            perror ( "[-] Noeud : probleme accept");
            noeud_exit(com_sockets, i);
        }
        else {
            com_sockets[i] = dsCv;
            accepted++;
        }
    }

    return accepted;
}

int send_msg(int *tab_sockets, int nb_sockets, void *msg, size_t msg_size){
    int sent = 0;
    for(int i=0; i<nb_sockets; i++){
        int s = send_tcp(tab_sockets[i], msg, msg_size);
        if(s < 0){
            perror("[-] Noeud : problem sending message.");
        }
        else sent++;
    }
    return sent;
}

int receive_msg(int *tab_sockets, int nb_sockets, size_t msg_size){
    void *msg = malloc(msg_size);
    int tmp = 0;
    int received = 0;

    for(int i =0; i < nb_sockets; i++){
        int rcv = receive_tcp(tab_sockets[i], msg, msg_size);
        if(rcv < 0){
            perror("[-] Noeud : problem receiving message");
            exit(1);
        } else received++;
        tmp = *((int *) msg);
        printf("message reçu du %d eme voisin : %d\n", i, tmp );
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


void append_arrays(int *res, int *tab1, int *tab2, int size1, int size2){
    for(int i=0; i<size1; i++){
        res[i] = tab1[i];
    }
    for(int i=0; i<size2; i++){
        res[size1+i] = tab2[i];
    }
}

void set_voisins(fd_set *set, int *tab1, int *tab2, int size1, int size2){
    for(int i=0; i<size1; i++){
        FD_SET(tab1[i], set);
    }
    for(int i=0; i<size2; i++){
        FD_SET(tab2[i], set);
    }
}

int choose_color(int *colors){
    for(int i=0; i<GRAPH_SIZE; i++){
        printf("couleur %d: %d\n", i, colors[i]);
    }
    for(int i=0; i<GRAPH_SIZE; i++){
        if(colors[i]) return i;
    }
    return -1;
}

void *send_color(void *arg){
    int *info = (int *) arg;
    printf("thread sending color: %d\n", info[0]);
    send_tcp(info[3], info, sizeof(int)*4);
    pthread_exit(NULL);
}

void broadcast_color(int *tab_sockets, int nb_sockets, int color){
    int info[4];
    info[0] = color;
    info[2] = INDICE;
    pthread_t threads[nb_sockets];

    for(int i=0; i<nb_sockets; i++){
        info[1] = i == 0;
        info[3] = tab_sockets[i];
        pthread_create(&threads[i], NULL, send_color, (void *) &info);
        pthread_join(threads[i], NULL);
    }
}

void receive_colors(fd_set *set, int *tab_sockets, int nb_sockets, int *colors){
    int info[3];
    int couleur;

    printf("[+] Noeud %d: je vais recevoir les couleurs de mes voisins\n", INDICE);

    select(FD_SETSIZE, set, NULL, NULL, NULL);
    for(int i =0; i < nb_sockets; i++){
        if(FD_ISSET(tab_sockets[i], set)){
            receive_tcp(tab_sockets[i], info, sizeof(info));
            couleur = info[0];
            colors[couleur] = 0;
            FD_CLR(tab_sockets[i], set);
            printf("[+] Noeud %d: j'ai reçu la couleur %d de mon voisin %d\n", INDICE, couleur, info[2]);
            printf("[+] Noeud %d: %s", INDICE, info[1] ? "je suis le prochain\n" : "j'attends mes autres voisins\n");
            if(info[1]) return;
        }
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

    SERVER_SOCKET = socket(AF_INET, SOCK_STREAM, 0);

    if (SERVER_SOCKET == -1){
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

    int conn = connect(SERVER_SOCKET,
                       (struct sockaddr*) &server_address,
                       lgAdr);
    if (conn < 0){
        perror("[-] Client: problème de connexion avec server");
        close(SERVER_SOCKET);
        exit(1);
    }
    printf("[+] Client: demande de connexion avec server reussie\n");

    int net_info[5];

    int rcv = receive_tcp(SERVER_SOCKET, net_info, sizeof(net_info));

    if (rcv < 0){
        perror ( "[-] Client: probleme de reception");
        close(SERVER_SOCKET);
        exit(1);
    }
    else if (rcv == 0)
    {
        printf("[-] Client: socket server fermée\n");
        close(SERVER_SOCKET);
        exit(1);
    }

    int in = net_info[0];
    int out = net_info[1];
    INDICE = net_info[2];
    GRAPH_SIZE = net_info[3];
    GRAPH_TYPE = net_info[4];


    struct sockaddr_in in_addresse;
    if(in > 0){
        create_in_socket(&in_addresse);
        mise_en_ecoute(&in_addresse, in);
    }

    int out_sockets[out];
    struct Noeud out_noeuds[out];
    create_out_sockets(out_sockets, out);
    printf("[+] Client: creation des sockets out OK\n");

    if(in > 0){
        int send_in = send_tcp(SERVER_SOCKET, &in_addresse, sizeof(struct sockaddr_in));
        if (send_in < 0){
            perror("[-] Client: probleme d'envoi des adresses in");
            noeud_exit(out_sockets, out);
            exit(1);
        }
        printf("[+] Client: envoi des adresses_in OK\n");
    }

    /* receive out addresses from server and assign them to out sockets */

    struct Noeud noeud;
    for(int i=0; i<out; i++){
        int rcv = receive_tcp(SERVER_SOCKET, &noeud, sizeof(struct Noeud));
        if (rcv < 0){
            perror ( "[-] Client: probleme de reception");
            noeud_exit(out_sockets, out);
            exit(1);
        }
        else if (rcv == 0)
        {
            printf("[-] Client: socket server fermée\n");
            noeud_exit(out_sockets, out);
            exit(1);
        }
        out_noeuds[i] = noeud;
    }

    printf("[+] Client: reception des adresses out OK\n");

    int connections = establish_connections(out_sockets, out_noeuds, out);
    printf("[+] Noeud %d: %d connexions sortantes établies\n", INDICE, connections);

    int communication_sockets[in];
    int accepted = accept_connections(communication_sockets, in);
    printf("[+] Noeud %d: %d connexions entrantes acceptées\n", INDICE, accepted);


    // Reseau cree, on peut commencer l'algo

    // 0 si graphe complet, (n couleurs)
    // 1 si graphe étoile,  (2 couleurs)
    // 2 si graphe cycle,   (2 couleurs) (3 si cycle impair)
    // 3 si graphe chemin,  (2 couleurs) (3 si erreur d'analyse cf return 3)
    // 4 si graphe aléatoire

    int couleurs[GRAPH_SIZE];
    for(int i=0; i<GRAPH_SIZE; i++){
        couleurs[i] = 1;
    }
    int couleur = INDICE - 1;

    int starts;
    receive_tcp(SERVER_SOCKET, &starts, sizeof(int));

    int voisins[in + out];
    append_arrays(voisins, communication_sockets, out_sockets, in, out);

    fd_set voisin_set;
    FD_ZERO(&voisin_set);
    set_voisins(&voisin_set, communication_sockets, out_sockets, in, out);

    switch (GRAPH_TYPE) {
        case 0:
            printf("[+] Noeud %d: le graphe est complet, algo inutile : \n", INDICE);
            printf("[+] Noeud %d: je termine avec la couleur %d\n", INDICE, couleur);
            send_tcp(SERVER_SOCKET, &couleur, sizeof(int));

            sleep(180);

            noeud_exit(voisins, in + out);

            return 0;
            break;
        case 1:
            printf("[+] Noeud %d: le graphe est une étoile\n", INDICE);
            if (starts) {
                printf("[+] Noeud %d: je suis la racine\n", INDICE);
                sleep(5);
                broadcast_color(voisins, in + out, couleur);

            } else {
                printf("[+] Noeud %d: je suis une extrémité\n", INDICE);
                receive_colors(&voisin_set, voisins, in + out, couleurs);
                couleur = choose_color(couleurs);
            }
            //pas besoin d'informer le voisin, j'en ai qu'un et c'est la racine
            break;
        case 2:
            printf("[+] Noeud %d: le graphe est un cycle\n", INDICE);
            if (starts) {
                printf("[+] Noeud %d: je suis le premier\n", INDICE);
                sleep(5);
                broadcast_color(voisins, in + out, couleur);

            } else {
                printf("[+] Noeud %d: je suis un noeud intermédiaire\n", INDICE);
                receive_colors(&voisin_set, voisins, in + out, couleurs);
                couleur = choose_color(couleurs);
                broadcast_color(voisins, in + out, couleur);
            }

            break;
        case 3:
            printf("[+] Noeud %d: le graphe est un chemin\n", INDICE);
            break;
        case 4:
            printf("[+] Noeud %d: le graphe est quelconque\n", INDICE);

            if (starts) {
                printf("[+] Noeud %d: je commence\n", INDICE);
                broadcast_color(voisins, in + out, couleur);
            } else {
                receive_colors(&voisin_set, voisins, in + out, couleurs);
                couleur = choose_color(couleurs);
                broadcast_color(voisins, in + out, couleur);
            }

            break;

    }

    printf("[+] Noeud %d: je termine avec la couleur %d\n", INDICE, couleur);

    send_tcp(SERVER_SOCKET, &couleur, sizeof(int));

    sleep(180);

    noeud_exit(voisins, in+out);
}

