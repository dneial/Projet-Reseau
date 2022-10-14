#!/bin/bash

if [ $# -ne 2 ]
then
    echo "[-] Indiquer le port du serveur et le nombre de clients"
else
    server=$1
    nb_clients=$2
    for(( c=1; c<=$nb_clients; c++))
    do
	port=$(($server + $c))
	echo "[+] Launching client: Socket d'entré @ $port"
	gnome-terminal --tab -- bin/client 0 $server $port &

    done
fi
