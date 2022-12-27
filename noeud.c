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
int MAX_FD;

struct Map{
    int socket;
    int indice;
    int etat;
};

int* get_sockets_from_map(struct Map* map, int mapSize){
    int *sockets = malloc(sizeof(int)*mapSize);
    for(int i = 0; i < mapSize; i++){
        sockets[i] = map[i].socket;
    }
    return sockets;
}


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

void create_out_sockets(struct Map *tab_sockets, int nb_sockets){
    int out_socket;
    for(int i=0; i<nb_sockets; i++){
        out_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (out_socket == -1){
            printf("[-] Client : pb creation socket sortie\n");
            noeud_exit(get_sockets_from_map(tab_sockets, i), i);
        }
        MAX_FD = MAX_FD > out_socket ? MAX_FD : out_socket;
        tab_sockets[i].socket = out_socket;
    }
}

void get_out_adresses(struct Noeud *noeuds, struct Map *tab_sockets, int nb_sockets){
    struct Noeud noeud;
    for(int i=0; i<nb_sockets; i++){
        int rcv = receive_tcp(SERVER_SOCKET, &noeud, sizeof(struct Noeud));
        if (rcv <= 0){
            perror ( "[-] Client: probleme de reception");
            noeud_exit(get_sockets_from_map(tab_sockets, nb_sockets+i), nb_sockets+i);
        }
        noeuds[i] = noeud;
        tab_sockets[i].indice = noeud.index;
        tab_sockets[i].etat = 0;
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
    IN_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
    if (IN_SOCKET == -1) {
        printf("[-] Client : pb creation socket sortie\n");
        close(SERVER_SOCKET);
        exit(1);
    }

    MAX_FD = IN_SOCKET;

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
        printf("\n[+] Noeud %d: je me connecte au noeud %d @ %d\n",
               INDICE, noeuds[i].index, ntohs(noeuds[i].addr.sin_port));
        if (connect(tab_sockets[i], (struct sockaddr *) &noeuds[i].addr, sizeof(struct sockaddr_in)) < 0) {
            perror("[-] Noeud : erreur connect.\n");
            noeud_exit(tab_sockets, i);
        }
        send_tcp(tab_sockets[i], &INDICE, sizeof(int));
        connections++;
    }
    printf("\n");
    return connections;
}

int accept_connections(struct Map *tab_voisins, int nb_sockets, int out){
    struct sockaddr_in adC ; // obtenir adresse client accepté
    socklen_t lgC = sizeof (struct sockaddr_in);
    int accepted = 0;
    int indice;

    for(int i=0; i<nb_sockets; i++){
        int dsCv = accept(IN_SOCKET,(struct sockaddr *) &adC, &lgC);
        if (dsCv < 0){
            perror ( "[-] Noeud : probleme accept");
            noeud_exit(get_sockets_from_map(tab_voisins, out + i), out + i);
        }
        else {
            receive_tcp(dsCv, &indice, sizeof(int));
            MAX_FD = MAX_FD > dsCv ? MAX_FD : dsCv;
            tab_voisins[out+i].socket = dsCv;
            tab_voisins[out+i].indice = indice;
            tab_voisins[out+i].etat = 0;
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


void set_voisins(fd_set *set, struct Map *tab_sockets, int nb_sockets){
    for(int i=0; i<nb_sockets; i++) {
        FD_SET(tab_sockets[i].socket, set);
    }
}

int choose_color(int *colors){
    for(int i=0; i<GRAPH_SIZE; i++){
        if(colors[i]) return i;
    }
    return -1;
}

void *send_color(void *arg){
    int *info = (int *) arg;
    send_tcp(info[3], info, sizeof(int)*3);
    pthread_exit(NULL);
}

void inform_parent(int parent_socket){
    int msg = 1;
    send_tcp(parent_socket, &msg, sizeof(int));
    printf("[+] Noeud %d: j'ai informé mon parent\n", INDICE);
}

void attend_fils(int fils_pos, struct Map *tab_voisins, int degre){
    int msg;
    receive_tcp(tab_voisins[fils_pos].socket, &msg, sizeof(int));
    tab_voisins[fils_pos].etat = 1;
}


//renvoie la position dans le tab_voisins, du prochain noeud à colorier
//(choisit le voisin non colorié avec le plus petit indice)
int get_prochain(struct Map *tab_voisins, int degre){
    int prochain = -1;
    int indice_prochain = GRAPH_SIZE+1;

    //PROBLEME ICI :
    //JE NE SAIS PAS SI UN FILS EN A DÉJA COLORIÉ UN AUTRE
    //DONC QUAND J'ENVOIE UN MESSAGE À L'AUTRE IL AURA PEUT-ÊTRE DÉJÀ TERMINÉ
    //ET CA VA ME BLOQUER JUSQU'À CE QUE SON TERMINAL FERME ?

    //  idée 1 : signal de départ = une liste de noeuds que je veux colorier moi même
    // + comme ça mes fils ne colorieront pas les noeuds de ma liste
    // - point négatif : ils doivent eux même envoyer leur liste perso + la mienne à leurs fils
    //ça peut faire des gros messages pas pratiques à envoyer

    //  idée 2 : dans l'autre sens :
    // + quand ils ont fini mes fils me disent qui à été colorié par eux et leurs fils
    // (+/-) je traite le gros message que chez le parent = je réduis le nombre de messages à envoyer
    //  - point négatif : toujours gros messages à envoyer

    //  idée 3 : mieux !!
    // A chaque fois qu'un fils m'informe qu'il a fini, je check si les restants
    // m'ont envoyé leur couleur entre temps
    //  => ça implique qu'on envoie sa couleur à tous nos voisins
    //  et pas uniquement ceux qui n'ont pas été traités.
    // problème : il faut pas rester bloquer dans la reception si on a pas reçu de couleur
    //  => Est-ce que FD_ISSET() est bloquant ?



    for(int i=0; i<degre; i++){
        if(tab_voisins[i].etat == 0 && tab_voisins[i].indice < indice_prochain) {
            prochain = i;
            indice_prochain = tab_voisins[i].indice;
        }
        //printf("\t[DEBUG] prochain = %d ; indice_prochain = %d ; i = %d\n",prochain, tab_voisins[prochain].indice, i );
    }
    //printf("\t[DEBUG] fin choix prochain = %d\n",prochain);
    return prochain;
}


/////////////////////////////////////////


    //reçoit les couleurs des voisins, met à jour le tableau des couleurs disponibles et l'etat des voisins
int receive_colors(fd_set *set, struct Map *tab_voisins, int degre, int *colors){
    int info[4];
    int couleur;
    int parent = -1;

    for(int n=0; n<degre; n++){
        for(int i=0; i<degre; i++){
            FD_SET(tab_voisins[i].socket, set);
        }
        printf("%d messages\n", select(MAX_FD+1, set, NULL, NULL, NULL));
        for(int i = 0; i < degre; i++){
            if(FD_ISSET(tab_voisins[i].socket, set)){
                printf("socket is set: %d\n", tab_voisins[i].socket);
                receive_tcp(tab_voisins[i].socket, info, sizeof(int)*3); //reception couleur
                if(info[0] != -1) {
                    couleur = info[0];
                    colors[couleur] = 0;
                    tab_voisins[i].etat = 1; //maj tableau
                    printf("[+] Noeud %d: j'ai reçu la couleur %d de mon voisin %d\n", INDICE, couleur, info[2]);
                }
                printf("[+] Noeud %d: %s", INDICE,
                       info[1] ? "je suis le prochain\n" : "j'attends mes autres voisins\n");

                if(info[1]) parent = tab_voisins[i].socket; //signal départ
            }
        }
        if(parent != -1) return parent;
    }
    return parent;
}


    //transmet notre couleur personelle à tous les voisins non encore coloriés
    //retourne la position (dans tab_voisin) du prochain voisin à colorier
int broadcast_color(struct Map *tab_voisins, int degre, int color){
    int info[4];
    info[0] = color;
    info[2] = INDICE;
    pthread_t threads[degre];
    int prochain = get_prochain(tab_voisins, degre);

    if(prochain == -1) {
        printf("[+] Noeud %d: il n'y a plus de voisins incolores\n", INDICE);
        return -1;
    }
    printf("[+] Noeud %d: le prochain est le noeud %d\n",INDICE, tab_voisins[prochain].indice);

    for (int i = 0; i < degre; i++) {
        if (tab_voisins[i].etat != 1) { //parmi les voisins non coloriés
            info[1] = i == prochain; //si le voisin est le prochain à faire tourner l'algorithme


            //  info a la socket car c'est l'arg de la fonction des threads, mais on l'envoie pas
            info[3] = tab_voisins[i].socket; //

            //printf("socket %d est la prochaine ? %d\n", tab_voisins[i], info[1]);
            printf("[+] Noeud %d: envoi de la couleur %d au voisin %d @ %d\n",INDICE, color, tab_voisins[i].indice,
                   tab_voisins[i].socket);
            pthread_create(&threads[i], NULL, send_color, (void *) &info);
            pthread_join(threads[i], NULL);
        }
    }
    return prochain;

}


    //envoie au prochain voisin à colorier le signal de départ de l'algorithme
    //attend son message de fin de traitement et recommence avec un autre jusqu'à ce que tous aient été coloriés
void boucle_fils(struct Map *tab_voisins, int degre, int fils) {

    //attendre le signal de fin du premier fils si j'en ai un
    if (fils > 0) {
        attend_fils(fils, tab_voisins, degre);
        printf("[+] Noeud %d: le fils %d a fini de colorier\n\n",INDICE, tab_voisins[fils].indice);


        //IDEE 3 : check si les autres fils ont fini entre temps, un genre de :
        //updateAutreFils(tab_voisins, degre);



        int info[3];
        info[0] = -1; //pour que le fils capte direct que c'est un signal de son parent
        info[1] = 1;
        info[2] = INDICE;

        while ((fils = get_prochain(tab_voisins, degre)) != -1) {
            printf("[+] Noeud %d: envoi du signal de départ au prochain (noeud %d)\n", INDICE, tab_voisins[fils].indice);
            send_tcp(tab_voisins[fils].socket, &info, sizeof(info));

            attend_fils(fils, tab_voisins, degre);
            printf("[+] Noeud %d: le fils %d a fini de colorier\n\n", INDICE, tab_voisins[fils].indice);
        }
        printf("[+] Noeud %d: tous mes fils ont fini de colorier\n",INDICE);
    }
}

void parse_args(int argc, char *argv[], int *verbose, int *network, char *server_ip, int *server_port){
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0){
            printf("[+] Client: verbose mode\n");
            *verbose = 1;

            if (argc > 2){
                if (strcmp(argv[2], "-n") == 0 || strcmp(argv[2], "--network") == 0){
                    printf("[+] Client: Network mode\n");
                    if (argc == 5){
                        *network = 1;
                        //tester si format valide normalement
                        strcpy(server_ip, argv[3]);
                        *server_port = atoi(argv[4]);
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
                *network = 1;
                //tester si format valide normalement
                strcpy(server_ip, argv[2]);
                *server_port = atoi(argv[3]);
            }
            else{
                printf("[-] Client: missing arguments for network mode\n");
                printf("[-] Utilisation: %s [-v|--verbose] [-n|--network <IP> <PORT>]\n", argv[0]);
                exit(1);
            }
            *network = 1;

            if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0){
                printf("[+] Client: verbose mode\n");
                *verbose = 1;
            }
        }
        else{
            printf("[-] Client: unknown option %s\n", argv[1]);
            exit(1);
        }
    }

    /* Creation socket Pour Server*/

    if (!*network)
    {
        printf("[+] Client: local mode\n");
        strcpy(server_ip, "0.0.0.0");
        *server_port = read_server_port();
    }

}

void connect_to_server(char *ip, int port){

    SERVER_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
    printf("server socket is %d\n", SERVER_SOCKET);

    if (SERVER_SOCKET == -1){
        printf("[-] Client: problème création socket\n");
        exit(1);
    }

    MAX_FD = SERVER_SOCKET;
    printf("[+] Client: création de la socket OK\n");

    /* contient adresse socket serveur */

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(server_address.sin_addr));
    server_address.sin_port = htons(port);

    socklen_t lgAdr = sizeof(struct sockaddr_in);


    printf("[+] Client: Je tente la connexion avec le serveur @ %s:%d\n",
           inet_ntoa(server_address.sin_addr), port);

    int conn = connect(SERVER_SOCKET,
                       (struct sockaddr*) &server_address,
                       lgAdr);
    if (conn < 0){
        perror("[-] Client: problème de connexion avec server");
        close(SERVER_SOCKET);
        exit(1);
    }
    printf("[+] Client: demande de connexion avec server reussie\n");

}

/////////////////////////////////////////


int main(int argc, char *argv[]) {

    if (argc > 5){
        printf("[-] Utilisation: %s [-v|--verbose] [-n|--network <IP> <PORT>]\n", argv[0]);
        exit(0);
    }

    int verbose = 0;
    int network = 0;
    char* server_ip = malloc(16);
    int server_port;

    parse_args(argc, argv, &verbose, &network, server_ip, &server_port);

    connect_to_server(server_ip, server_port);

    int net_info[5];
    int rcv = receive_tcp(SERVER_SOCKET, net_info, sizeof(net_info));
    if (rcv <= 0){
        perror ( "[-] Client: probleme de reception");
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
        int send_in = send_tcp(SERVER_SOCKET, &in_addresse, sizeof(struct sockaddr_in));
        if (send_in < 0){
            perror("[-] Client: probleme d'envoi des adresses in");
            close(SERVER_SOCKET);
            close(IN_SOCKET);
            exit(1);
        }
        printf("[+] Client: envoi de l'adresse d'entrée OK\n");
    }

    struct Noeud noeuds[out];

    int degre = in+out;
    struct Map tab_voisins[degre];

    create_out_sockets(tab_voisins, out);
    printf("[+] Client: creation des sockets out OK\n");


    /* receive out addresses from server and assign them to out sockets */
    get_out_adresses(noeuds, tab_voisins, out);
    printf("[+] Client: reception des adresses out OK\n");

    int connections = establish_connections(get_sockets_from_map(tab_voisins, out), noeuds, out);
    printf("[+] Noeud %d: %d connexions sortantes établies\n", INDICE, connections);

    int accepted = accept_connections(tab_voisins, in, out);
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

/*
    int nb_restants = degre;

    int voisins[degre]; //tous es voisins
    int voisins_restants[degre]; //voisins restants à colorier
    append_arrays(voisins, out_sockets, communication_sockets, out, in);
    append_arrays(voisins_restants, voisins, NULL, degre, 0);*/

    fd_set voisin_set;
    FD_ZERO(&voisin_set);
    set_voisins(&voisin_set, tab_voisins, degre);

    int parent = -1;
    int fils = -1;

    for(int i=0; i<degre; i++){
        printf("socket: %d voisin: %d\n", tab_voisins[i].socket, tab_voisins[i].indice);
    }
    switch (GRAPH_TYPE) {
        case 0:
            printf("[+] Noeud %d: le graphe est complet, algo inutile : \n", INDICE);
            printf("[+] Noeud %d: je termine avec la couleur %d\n", INDICE, couleur);
            send_tcp(SERVER_SOCKET, &couleur, sizeof(int));

            sleep(180);

            noeud_exit(get_sockets_from_map(tab_voisins, degre), degre);


            return 0;
            break;
        case 1:
            printf("[+] Noeud %d: le graphe est une étoile\n", INDICE);
            if (starts) {
                printf("[+] Noeud %d: je suis la racine\n", INDICE);
                broadcast_color(tab_voisins, degre, couleur);

            } else {
                printf("[+] Noeud %d: je suis une extrémité\n", INDICE);
                receive_colors(&voisin_set, tab_voisins, degre, couleurs);
                couleur = choose_color(couleurs);
            }
            //pas besoin d'informer le voisin, j'en ai qu'un et c'est la racine
            break;
        case 2:
            printf("[+] Noeud %d: le graphe est un cycle\n", INDICE);
            if (starts) {
                printf("[+] Noeud %d: je suis le premier\n", INDICE);
                fils = broadcast_color(tab_voisins, degre, couleur);
                //attend_fils(fils);

            } else {
                printf("[+] Noeud %d: je suis un noeud intermédiaire\n", INDICE);
                parent = receive_colors(&voisin_set, tab_voisins, degre, couleurs);
                couleur = choose_color(couleurs);
                fils = broadcast_color(tab_voisins, degre, couleur);
                //attend_fils(fils);
                inform_parent(parent);
            }

            break;
        case 3:
            printf("[+] Noeud %d: le graphe est un chemin\n", INDICE);
            break;
        case 4:
            printf("[+] Noeud %d: le graphe est quelconque\n\n", INDICE);

            if (degre==0){
                printf("[+] Noeud %d: je suis un noeud isolé\n", INDICE);
                couleur = 0;
            } else if (starts) {
                printf("[+] Noeud %d: je commence\n", INDICE);
                couleur = choose_color(couleurs);
                printf("[+] Noeud %d: je choisis la couleur %d\n", INDICE, couleur);
                fils = broadcast_color(tab_voisins, degre, couleur);
                boucle_fils(tab_voisins, degre, fils);
            } else {
                parent = receive_colors(&voisin_set, tab_voisins, degre, couleurs);
                couleur = choose_color(couleurs);
                fils = broadcast_color(tab_voisins, degre, couleur);
                boucle_fils(tab_voisins, degre, fils);
                inform_parent(parent);
            }
            break;

    }

    printf("[+] Noeud %d: je termine avec la couleur %d\n", INDICE, couleur);

    send_tcp(SERVER_SOCKET, &couleur, sizeof(int));
    printf("[+] Noeud %d: j'ai envoyé ma couleur au serveur\n", INDICE);

    sleep(180);

    noeud_exit(get_sockets_from_map(tab_voisins, degre), degre);
}

