//
// Created by daniel.azevedo-gomes@etu.umontpellier.fr on 17/11/22.
//

#ifndef PROJET_RESEAU_READ_GRAPH_H
#define PROJET_RESEAU_READ_GRAPH_H

struct Edge {
    int v1;
    int v2;
};

struct Graph {
    int sommets;
    int aretes;
    int **matrix;
};

void read_headers(FILE *file, int print_flag);

void read_graph_info(FILE *file, struct Graph *graph);

void read_edge_info(FILE *file, struct Edge *edge);

void create_matrix(struct Graph *graph);

void free_matrix(struct Graph *graph);

void read_graph(FILE *file, struct Graph *graph);

void print_matrix(struct Graph *graph);

#endif //PROJET_RESEAU_READ_GRAPH_H
