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
	@echo "\n Compilation terminée avec succès !"
	@echo "\n Commandes disponibles :"
	@echo "   make run-server   : Lancer le serveur sur 127.0.0.1:8082"
	@echo "   make run-client   : Lancer un client"
	@echo "   make clean        : Nettoyer tous les fichiers générés"
	@echo "   make help         : Afficher toutes les commandes"
	@echo "\n Pour démarrer rapidement :"
	@echo "   1. Ouvrir un terminal : make run-server"
	@echo "   2. Ouvrir un autre terminal : make run-client"

# Compilation du serveur TCP
serverTCP: serverTCP.c common.c common.h
	@echo " Compilation du serveur..."
	$(CC) $(CFLAGS) -o serverTCP serverTCP.c common.c

# Compilation du client TCP  
clientTCP: clientTCP.c common.c common.h
	@echo " Compilation du client..."
	$(CC) $(CFLAGS) -o clientTCP clientTCP.c common.c

# Arrêter le serveur 
stop-server:
	@echo " Arrêt du serveur..."
	@-pkill -f "serverTCP" 2>/dev/null || true
	@echo " Serveur arrêté."

# Nettoyage du projet
clean: stop-server
	@echo " Nettoyage des fichiers générés..."
	# Suppression des exécutables
	rm -f serverTCP clientTCP
	# Suppression des fichiers de configuration serveur
	rm -f server_info.txt users.txt
	# Suppression de tous les dossiers de logs de test
	rm -rf logs_test_*
	@echo " Nettoyage terminé !"

# Lancement du serveur
run-server: serverTCP
	@echo " Lancement du serveur sur 127.0.0.1:8082..."
	@echo " Info : Gardez ce terminal ouvert"
	@echo "   Pour arrêter : Ctrl+C, puis 'make stop-server'"
	@echo "   Pour lancer un client : ouvrir un autre terminal et 'make run-client'"
	@echo "=================================================="
	./serverTCP 127.0.0.1 8082

# Lancement d'un client
run-client: clientTCP
	@echo " Lancement du client..."
	@echo " Info : Le client se connectera au serveur"
	@echo "   Assurez-vous que le serveur est déjà lancé !"
	@echo "=================================================="
	./clientTCP

# Afficher l'aide
help:
	@echo "\n AIDE - Makefile Client/Serveur TCP"
	@echo "========================================"
	@echo " Compilation :"
	@echo "   make all        : Compile serveur ET client (cible par défaut)"
	@echo "   make serverTCP  : Compile uniquement le serveur"
	@echo "   make clientTCP  : Compile uniquement le client"
	@echo ""
	@echo " Exécution :"
	@echo "   make run-server : Lance le serveur (127.0.0.1:8082)"
	@echo "   make run-client : Lance un client"
	@echo ""
	@echo " Arrêt :"
	@echo "   make stop-server: Arrête le serveur en cours d'exécution"
	@echo ""
	@echo " Nettoyage :"
	@echo "   make clean      : Supprime exécutables, logs et fichiers temporaires"
	@echo ""
	@echo " Informations :"
	@echo "   make help       : Affiche ce message d'aide"
	@echo ""
	@echo " Procédure recommandée :"
	@echo "   1. Terminal 1 : make run-server"
	@echo "   2. Terminal 2 : make run-client"
	@echo "   3. Pour arrêter : Ctrl+C dans les deux terminaux"

# Déclaration des cibles phony
.PHONY: all clean run-server run-client stop-server help