# Makefile pour le projet Client/Serveur TCP
# Compilation et gestion des tests multiclients

# Compilateur
CC = gcc

# Options de compilation
# -Wall : Active tous les warnings
# -g    : Génère les informations de débogage
CFLAGS = -Wall -g

# Cible principale : compile le serveur et le client
all: serverTCP clientTCP

# Compilation du serveur TCP
# Dépend des fichiers serverTCP.c, common.c et common.h
serverTCP: serverTCP.c common.c common.h
	$(CC) $(CFLAGS) -o serverTCP serverTCP.c common.c

# Compilation du client TCP  
# Dépend des fichiers clientTCP.c, common.c et common.h
clientTCP: clientTCP.c common.c common.h
	$(CC) $(CFLAGS) -o clientTCP clientTCP.c common.c

# Nettoyage du projet
# Supprime les exécutables, fichiers temporaires et dossiers de logs
clean:
	# Suppression des exécutables
	rm -f serverTCP clientTCP
	
	# Suppression des fichiers de configuration serveur
	rm -f server_info.txt users.txt
	
	# Suppression de tous les dossiers de logs de test
	# -r : récursif, -f : forcé sans confirmation
	rm -rf logs_test_*
	
	# Message de confirmation
	@echo " Nettoyage terminé - exécutables, fichiers config et logs supprimés"

# Lancement du serveur
# Prérequis : le serveur doit être compilé
run-server: serverTCP
	./serverTCP 127.0.0.1 8082

# Lancement d'un client
# Prérequis : le client doit être compilé  
run-client: clientTCP
	./clientTCP

# Déclaration des cibles phony (ne correspondent pas à des fichiers)
.PHONY: all clean run-server run-client