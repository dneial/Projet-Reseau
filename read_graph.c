#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define EOL '\n'
#define SEPARATOR ' '
#define NB_TOKENS 4
#define SIZE_TOKENS 32

struct Graph {
  int vertices;
  int edges;
};

struct Edge {
  int v1;
  int v2;
};

char read_headers(FILE *file){
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
  graph->vertices = atoi(graph_info);
 
  
  getdelim(&graph_info, &n, 32, file);
  graph->edges = atoi(graph_info);

}

void read_edge_info(FILE *file, struct Edge *edge){
  char *edge_info;
  size_t n;

  getdelim(&edge_info, &n, 32, file);
  edge->v1 = atoi(edge_info);

  getdelim(&edge_info, &n, 32, file);
  edge->v2 = atoi(edge_info);

}

int** create_matrix(FILE *file, int nb_edges, int nb_vertices){
  int *values = calloc(nb_edges*nb_edges, sizeof(int));
  int **matrix = malloc(nb_edges*sizeof(int*));
  struct Edge e;
  
  for(int i=0; i<nb_vertices; i++){
    read_edge_info(file, &e);
    values[e.v2-1] = 1;
    matrix[i] = values + e.v1-1;
  }
  return matrix;
}



int main(int argc, char *argv[]){
  if(argc != 2){
    printf("Usage: %s <graph_file>\n", argv[0]);
    exit(1);
  }
  
  const char *FILENAME = argv[1];
  printf("Reading %s...\n", FILENAME);

  FILE *f = fopen(FILENAME, "r");
  struct Graph graph;

  // read headers 
  read_headers(f);

  read_graph_info(f, &graph);
  printf("Graph: %d vertices and %d edges\n", graph.vertices, graph.edges);

  int **m = create_matrix(f, graph.edges, graph.vertices);
  printf("%d\n", m[0][0]);
  exit(0);
  int matrix[graph.vertices][graph.vertices];

  for(int i = 0; i<graph.vertices; i++){
    memset(matrix[i], 0, graph.vertices); 
  }
  
  
  struct Edge e;
  for(int i=1; i<=graph.edges; i++){  
    read_edge_info(f, &e);
        
    matrix[e.v1-1][e.v2-1] = 1;
    
  }
  
   exit(0);
}
