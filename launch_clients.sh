#!/bin/bash

if [ $# -ne 1 ]
then
    echo "[-] Indiquer le nombre de clients"
else
    file="./c"
    nb_clients=$1
    for(( c=1; c<="$nb_clients"; c++))
    do
  	  gnome-terminal --tab -- "$file"
    done
fi
