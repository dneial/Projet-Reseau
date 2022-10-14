########################################
#~ définitions
########################################

# nom de l'executable
#BIN=runPeriod

BIN=bin/client  bin/serveur
#BIN=bin/serveur

# liste des fichiers sources 
SRCS0=server.c
SRCS1=client.c

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

clean:
	rm -f $(BIN) obj/*.o *~
