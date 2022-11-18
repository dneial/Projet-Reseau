########################################
#~ définitions
########################################

# nom de l'executable
#BIN=runPeriod

BIN=bin/serveur bin/noeud
#BIN=bin/serveur

# liste des fichiers sources 
SRCS0=server.c
SRCS1=noeud.c
SRCS2=tcp_communication.c
SRCS3=read_graph.c

SERVER_DEPS=read_graph.o tcp_communication.o
NOEUD_DEPS=tcp_communication.o

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

#bin/serveur: $(SRCS0:%.c=obj/%.o) $(SRCS2:%.c=obj/%.o) $(SRCS3:%.c=obj/%.o)
#	gcc -lpthread -o $@ $+ -g
#
#bin/noeud: $(SRCS1:%.c=obj/%.o) $(SRCS3:%.c=obj/%.o)
#	gcc -lpthread -o $@ $+ -g


clean:
	rm -f $(BIN) obj/*.o *~
