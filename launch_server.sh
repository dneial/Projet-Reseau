#!/bin/bash
if [ $# -ne 1 ]
then
    echo "Indiquer le ficher de graphe"
else
    echo "[1] Compiling"
    make
    graphe=$1
    file="bin/serveur"
    echo "[2] Launching server"
    echo ""
    gnome-terminal --tab -- "$file" "$graphe"
fi
