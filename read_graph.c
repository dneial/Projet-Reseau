#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_graph.h"



void read_headers(FILE *file, int print_flag){
    char *lines;
    size_t n;

  char next = fgetc(file);

    while(next != 'p'){
        getline(&lines, &n, file);
        next = fgetc(file);
        if(print_flag) printf("%s", lines);
    }

}


void read_graph_info(FILE *file, struct Graph *graph){
  char *graph_info;
  size_t n;

    //lit P
    getdelim(&graph_info, &n, 32, file);

    //lit edge
    getdelim(&graph_info, &n, 32, file);

    //lit le nb de sommets
    getdelim(&graph_info, &n, 32, file);
    graph->sommets = atoi(graph_info);

    //lit le nb d'aretes
    getdelim(&graph_info, &n, 32, file);
    graph->aretes = atoi(graph_info);

}

void read_edge_info(FILE *file, struct Edge *edge){
    char *edge_info;
    size_t n;

    getdelim(&edge_info, &n, 32, file);
    edge->v1 = atoi(edge_info);

    getdelim(&edge_info, &n, 32, file);
    edge->v2 = atoi(edge_info);


}

void create_matrix(struct Graph *graph){
    int nb_sommets = graph->sommets;
    graph->matrix = (int **)malloc(nb_sommets * sizeof(int *));
    for(int i = 0; i < nb_sommets; i++) {
        graph->matrix[i] = (int *) malloc(nb_sommets * sizeof(int));
    }

}

void free_matrix(struct Graph *graph){
    int nb_sommets = graph->sommets;
    for(int i = 0; i < nb_sommets; i++) {
        free(graph->matrix[i]);
    }
    free(graph->matrix);
}

void read_graph(FILE *file, struct Graph *graph){
    struct Edge e;
    for(int i=0; i<graph->aretes; i++){
        read_edge_info(file, &e);
        if(!graph->matrix[e.v2-1][e.v1-1]) graph->matrix[e.v1-1][e.v2-1] = 1;
    }
}

void print_matrix(struct Graph *graph){
    int nb_sommets = graph->sommets;
    printf("------\n");
    for(int i=0; i<nb_sommets; i++){
        for(int j=0; j<nb_sommets; j++){
            printf("%d ", graph->matrix[i][j]);
        }
        printf("\n");
    }
    printf("------\n");
}
