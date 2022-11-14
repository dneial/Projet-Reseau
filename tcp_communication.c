#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



int receive_tcp(int socket, void *buffer, size_t buffer_size){
    int current_size = buffer_size;
    int received = recv(socket, buffer, current_size, 0);

    while(received < buffer_size){
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