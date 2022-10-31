#!/bin/bash
if [ $# -ne 1 ]
then
    echo "Indiquer le ficher de graphe"
else
    echo "[+] Compiling"
    graphe=$1
    file="./server_port.txt"
    echo "[+] Launching server"
    echo ""
    gnome-terminal --tab -- ./s "$graphe"
    echo "[+] Launching clients"
    while IFS= read -r line;
    do
      echo "line: $line";
    done < "$file"
fi
