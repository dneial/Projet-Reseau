#!/bin/bash

if [ $# -lt 1 ]
then
    echo "[-] Utilisation: %s <NOMBRE DE CLIENTS> [-v|--verbose] [-n|--network <IP> <PORT>]"
else
    file="bin/noeud"
    nb_clients=$1
    ARG1=${2:-}
    ARG2=${3:-}
    ARG3=${4:-}
    ARG4=${5:-}

    if [ "$ARG1" = "-n" ] || [ "$ARG1" = "--network" ]
    then

      if [ "$ARG2" = "" ] || [ "$ARG3" = "" ]
      then
        echo "[-] in network mode both <IP> and <PORT> are compulsory"
        echo "[-] Utilisation: %s <NOMBRE DE CLIENTS> [-v|--verbose] [-n|--network <IP> <PORT>]"
        exit 1
      fi

      echo "[2] Launching clients in network mode"
      for(( c=1; c<="$nb_clients"; c++))
      do
          gnome-terminal --tab -- "$file" "$ARG1" "$ARG2" "$ARG3" "$ARG4"
      done
    else
      echo "[2] Launching clients"
      for(( c=1; c<="$nb_clients"; c++))
      do
  	      gnome-terminal --tab -- "$file"
      done
    fi
fi
