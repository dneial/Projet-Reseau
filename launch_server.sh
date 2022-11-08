#!/bin/bash
if [ $# -ne 1 ]
then
    echo "Indiquer le ficher de graphe"
else
    echo "[+] Compiling"
    make
    graphe=$1
    file="bin/serveur"
    port_file="./server_port.txt"
    echo "[+] Launching server"
    echo ""
    gnome-terminal --tab -- "$file" "$graphe"
    echo "[+] Launching noeuds"
    while IFS= read -r line;
    do
      echo "line: $line";
    done < "$port_file"
fi
