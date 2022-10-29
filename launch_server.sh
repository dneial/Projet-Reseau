#!/bin/bash
if [ $# -ne 2 ]
then
    echo "Indiquer le ficher de graphe"
else
    echo "[+] Compiling"
    make
    port=$1
    graphe=$2
    echo "[+] Launching server"
    echo ""
    gnome-terminal --tab -- ./bin/serveur $port $graphe
    echo "[+] Launching clients"
    ./launch_clients.sh
fi
