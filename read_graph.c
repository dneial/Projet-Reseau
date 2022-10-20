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


int read_graph_info(FILE *file, struct Graph *graph){
  char tokens[NB_TOKENS][SIZE_TOKENS];
  
  int j;
  char c;

  
  for(int i=0; i<NB_TOKENS; i++){
    j = 0;
    memset(tokens[i], 0, SIZE_TOKENS);
    c = fgetc(file);
    while (c != SEPARATOR){
      tokens[i][j++] = c;
      c = fgetc(file);
    }
  }
  
  int vertices = atoi(tokens[2]);
  int edges = atoi(tokens[3]);

  graph->vertices = vertices;
  graph->edges = edges;

}

void read_edge_info(FILE *file, struct Edge *edge){
  char *edge_info;
  size_t n;

  getline(&edge_info, &n, file);
  

  printf("%s", edge_info);
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

  for(int i=1; i<=5; i++){
    struct Edge e;
    read_edge_info(f, &e);
    if(fgetc(f) != 'e') break;
    fgetc(f);
    //    matrix[e1.v1][e1.v2] = 1;
  }
  
  

  

  //for(int i=0; i<graph.edges; i++){
  //  int v1, v2, j;
  //  char edge[3][10];
    


  //}
  
   exit(0);
}
