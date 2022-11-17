# README

## Serveur
Dans un terminal, depuis le dossier source de l’application, exécuter la commande :

`./launch_server.sh  <chemin_du_fichier_decrivant_le_graphe>`

Cette commande compile les fichiers sources puis lance le serveur avec pour argument le fichier du graphe à reproduire. 
Quelques fichiers de graphes sont fournis dans le répertoire `./graph_files`.

## Client
Une fois le serveur lancé et prêt (message : “[+] Server: j'attends la demande d'un client”) vous pouvez exécuter la commande :

`./launch_clients.sh <nombre_de_clients_à_lancer>`

Le nombre de clients à fournir à la commande est indiqué par le serveur à la deuxième ligne du terminal dans lequel il est lancé, 
ou dans le fichier de description de graphe, à la ligne préfixée d’un “p”.
l’application ouvrira autant de terminaux que de clients/nœuds.

Un Noeud enverra le message “hello” à tous ses voisins sortants et en recevra de tous ses voisins entrants.

Les terminaux se fermeront automatiquement au bout de 3 minutes après la fin de leur exécution.
