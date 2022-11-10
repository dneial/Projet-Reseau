########################################
#~ définitions
########################################

# nom de l'executable
#BIN=runPeriod

BIN=bin/noeud  bin/serveur
#BIN=bin/serveur

# liste des fichiers sources 
SRCS0=server.c
SRCS1=noeud.c
SRCS2=read_graph.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@

bin/serveur: $(SRCS0:%.c=obj/%.o)
	gcc -lpthread -o $@ $+ -g

bin/noeud: $(SRCS1:%.c=obj/%.o)
	gcc -lpthread -o $@ $+ -g

bin/read_graph: $(SRCS2:%.c=obj/%.o)
	gcc -lpthread -o $@ $+ -g

clean:
	rm -f $(BIN) obj/*.o *~
