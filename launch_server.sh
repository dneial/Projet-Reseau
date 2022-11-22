#!/bin/bash
if [ $# -lt 1 ]
then
#    echo "Indiquer le ficher de graphe"
    echo "[-] Utilisation: %s <GRAPH FILE> [-v|--verbose] [-n|--network]"
else
    echo "[1] Compiling"
    make
    graphe=$1
    ARG1=${2:-}
    ARG2=${3:-}
    file="bin/serveur"
    ip=$(hostname -I)
    echo ""
    if [ "$ARG1" = "-n" ] || [ "$ARG1" = "--network" ]
    then
        echo "[3] Launching server in network mode"
        gnome-terminal --tab -- "$file" "$graphe" "$ARG1" "$ARG2" "$ip"
    else
        echo "[3] Launching server "
        gnome-terminal --tab -- "$file" "$graphe"
    fi
fi
