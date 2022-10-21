#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Edge {
  int v1;
  int v2;
};

struct Graph {
  int sommets;
  int aretes;
  struct Edge *e;
};



void read_headers(FILE *file){
  char *lines;
  size_t n;

  char next = fgetc(file);

  while(next != 'p'){
    getline(&lines, &n, file);
    next = fgetc(file);
    printf("%s", lines);
  }

}


void read_graph_info(FILE *file, struct Graph *graph){
  char *graph_info;
  size_t n;

  getdelim(&graph_info, &n, 32, file);

  getdelim(&graph_info, &n, 32, file);

  getdelim(&graph_info, &n, 32, file);
  graph->sommets = atoi(graph_info);


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

int ** create_matrix(int nb_sommets){
  int *values = calloc(nb_sommets*nb_sommets, sizeof(int));
  int **matrix = malloc(nb_sommets*sizeof(int*));

  for(int i=0; i<nb_sommets; i++){
    matrix[i] = values + i*nb_sommets;
  }

  return matrix;
}

void fill_matrix(FILE *f, int nb_aretes, int nb_sommets, int m[nb_sommets][nb_sommets]){
  struct Edge e;
  for(int i=0; i<nb_aretes; i++){
    read_edge_info(f, &e);
    m[e.v1-1][e.v2-1] = 1;
  }
}


void read_graph(FILE *f, struct Graph *graph){

  struct Edge e;
  for(int i=0; i<graph->aretes; i++){
    read_edge_info(f, &e);
    graph->e[i] = e;
  }

}

/*
int main(int argc, char *argv[]){
  if(argc != 2){
    printf("Usage: %s <graph_file>\n", argv[0]);
    exit(1);
  }

  const char *FILENAME = argv[1];
  printf("Reading %s...\n", FILENAME);

  FILE *f = fopen(FILENAME, "r");
  struct Graph graph;

  read_headers(f);
  read_graph_info(f, &graph);
  struct Edge aretes[graph.aretes];
  graph.e = aretes; 

  read_graph(f, (struct Graph *) &graph);
  printf("Graph: %d sommets and %d aretes\n", graph.sommets, graph.aretes);

  exit(0);

}
*/