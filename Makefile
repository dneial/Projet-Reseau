########################################
#~ définitions
########################################

# nom de l'executable
#BIN=runPeriod

BIN=bin/client  bin/serveur  bin/read_graph
#BIN=bin/serveur

# liste des fichiers sources 
SRCS0=server.c
SRCS1=client.c
SRCS2=read_graph.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@

bin/serveur: $(SRCS0:%.c=obj/%.o)
	gcc -lpthread -o $@ $+

bin/client: $(SRCS1:%.c=obj/%.o)
	gcc -lpthread -o $@ $+

bin/read_graph: $(SRCS2:%.c=obj/%.o)
	gcc -lpthread -o $@ $+

clean:
	rm -f $(BIN) obj/*.o *~
