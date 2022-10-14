#!/bin/bash
if [ $# -ne 2 ]
then
    echo "Indiquer le port et le nombre de clients"
else
    echo "[+] Compiling"
    make
    port=$1
    nb_clients=$2
    echo "[+] Launching server @ address 0:$port, waiting for $nb_clients clients"
    echo ""
    gnome-terminal --tab -- ./bin/serveur $port $nb_clients
    echo "[+] Launching clients"
    ./launch_clients.sh $port $nb_clients 
fi
