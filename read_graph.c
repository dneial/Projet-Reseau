#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Graph {
  int sommets;
  int aretes;
};

struct Edge {
  int v1;
  int v2;
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
  printf("Graph: %d sommets and %d aretes\n", graph.sommets, graph.aretes);

  int matrix[graph.sommets][graph.sommets];
  struct Edge aretes[graph.aretes];



  for(int i = 0; i<graph.sommets; i++){
    memset(matrix[i], 0, graph.sommets);
  }

  struct Edge e;
  for(int i=0; i<graph.aretes; i++){
    read_edge_info(f, &e);
    aretes[i] = e;
    matrix[e.v1-1][e.v2-1] = 1;
  }

  // check
  int cpt = 0;
  for(int i=0; i<graph.sommets; i++){
    for(int j=0; j<graph.sommets; j++) {
      if(matrix[i][j]) cpt++;
  /*
      for(int a=0; a<graph.aretes; a++){
        if(aretes[a].v1 != i+1 || aretes[a].v2 != j+1)
          if(matrix[i][j]) printf("wrong!\n");
      }
    }
  */

    }
  }

  printf("ok? %s\n", cpt == graph.aretes ? "yes" : "no");
  exit(0);

}
