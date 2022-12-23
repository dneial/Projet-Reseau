//
// Created by daniel.azevedo-gomes@etu.umontpellier.fr on 17/11/22.
//
#ifndef PROJET_RESEAU_TCP_COMMUNICATION_H
#define PROJET_RESEAU_TCP_COMMUNICATION_H

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>

struct Noeud {
    struct sockaddr_in addr;
    int index;
};

int receive_tcp(int socket, void *buffer, size_t buffer_size){
    //printf("\ndebug: receive_tcp");
    int current_size = buffer_size;
    //printf("DEBUG : buffer size = %ld\n", buffer_size);
    int received = recv(socket, buffer, current_size, 0);
    //printf("DEBUG : received = %d\n", received);

    while(received < buffer_size){
      //  printf("DEBUG : while \n");
        if(received == 0){
            perror("Receive error");
            exit(1);
        }
        current_size -= received;
        buffer += received;
        received += recv(socket, buffer, current_size, 0);
    }

    return received;

}

int send_tcp(int socket, void *buffer, size_t buffer_size){
    int current_size = buffer_size;
    int sent = send(socket, buffer, current_size, 0);

    while(sent < buffer_size){
        if(sent == 0){
            perror("Receive error");
            exit(1);
        }
        current_size -= sent;
        buffer += sent;
        sent += send(socket, buffer, current_size, 0);
    }

    return sent;
}

#endif //PROJET_RESEAU_TCP_COMMUNICATION_H
