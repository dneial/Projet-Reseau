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





int main(int argc, char *argv[]){
  if(argc != 2){
    printf("Usage: %s <graph_file>\n", argv[0]);
    exit(1);
  }
  
  const char *FILENAME = argv[1];
  printf("Reading %s...\n", FILENAME);

  FILE *f = fopen(FILENAME, "r");
  struct Graph graph;
  struct Edge e1;
  // read headers 
  
  read_headers(f);
  read_graph_info(f, &graph);
  
  printf("Graph:\n%d vertices and %d edges\n", graph.vertices, graph.edges);

  int matrix[graph.vertices][graph.vertices];

  for(int i = 0; i<graph.vertices; i++){
    memset(matrix[i], 0, graph.vertices); 
  }
  printf("size of matrice: %ld\n", sizeof(matrix[0]));
  
  struct Edge e;
  for(int i=1; i<=12412; i++){  
    read_edge_info(f, &e);
        
    printf("Edge: %d -> %d\n", e.v1, e.v2);
    matrix[e.v1][e.v2] = 1;
    
  }

  //print_matrix(matrix);
  
  

  

  //for(int i=0; i<graph.edges; i++){
  //  int v1, v2, j;
  //  char edge[3][10];
    


  //}
  
   exit(0);
}
