//
// Created by daniel.azevedo-gomes@etu.umontpellier.fr on 17/11/22.
//

#ifndef PROJET_RESEAU_TCP_COMMUNICATION_H
#define PROJET_RESEAU_TCP_COMMUNICATION_H

int receive_tcp(int socket, void *buffer, size_t buffer_size);
int send_tcp(int socket, void *buffer, size_t buffer_size);

#endif //PROJET_RESEAU_TCP_COMMUNICATION_H
