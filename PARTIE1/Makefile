# Makefile pour compiler le client et le serveur UDP
# Utilisation: make        (compile tout)
#              make clean   (nettoie les fichiers compilés + tue le serveur)
#              make client  (compile seulement le client)
#              make server  (compile seulement le serveur)
#              make run-server ADDR=127.0.0.1 PORT=8080  (lance le serveur)
#              make run-client (lance le client)

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g

# Noms des exécutables
CLIENT = clientUDP
SERVER = serverUDP

# Port par défaut pour la détection des processus
DEFAULT_PORT = 8080

# Règle par défaut: compile tout
all: $(CLIENT) $(SERVER)
	@echo " Compilation terminée avec succès!"
	@echo ""
	@echo " Utilisation:"
	@echo "  Serveur: ./$(SERVER) <adresse> <port>"
	@echo "  Client:  ./$(CLIENT)"
	@echo ""
	@echo " Commandes rapides:"
	@echo "  make run-server ADDR=127.0.0.1 PORT=8080"
	@echo "  make run-client"

# Compilation du client
$(CLIENT): clientUDP.c common.h
	$(CC) $(CFLAGS) -o $(CLIENT) clientUDP.c
	@echo " Client compilé: ./$(CLIENT)"

# Compilation du serveur
$(SERVER): serverUDP.c common.h
	$(CC) $(CFLAGS) -o $(SERVER) serverUDP.c
	@echo " Serveur compilé: ./$(SERVER)"

# Règles individuelles
client: $(CLIENT)

server: $(SERVER)

# Arrêter le serveur 
stop-server:
	@echo " Arrêt du serveur UDP..."
	@-pkill -f "$(SERVER)" 2>/dev/null || true
	@-netstat -ano | grep ":$(DEFAULT_PORT) " | awk '{print $$5}' | while read pid; do \
		echo "  Arrêt du processus PID: $$pid"; \
		taskkill /F /PID $$pid 2>/dev/null || true; \
	done
	@echo " Serveur arrêté."

# Exécution du serveur avec paramètres
run-server: $(SERVER) stop-server
	@if [ -z "$(ADDR)" ] || [ -z "$(PORT)" ]; then \
		echo " Usage: make run-server ADDR=127.0.0.1 PORT=8080"; \
		echo "   Exemple: make run-server ADDR=127.0.0.1 PORT=5000"; \
		exit 1; \
	fi
	@echo " Lancement du serveur sur $(ADDR):$(PORT)..."
	./$(SERVER) $(ADDR) $(PORT)

# Exécution du client
run-client: $(CLIENT)
	@echo " Lancement du client..."
	./$(CLIENT)

# Nettoyage des fichiers compilés + arrêt du serveur
clean: stop-server
	@echo " Nettoyage des fichiers compilés..."
	rm -f $(CLIENT) $(SERVER) server_info.txt
	@echo "  Fichiers nettoyés"

# Nettoyage forcé
clean-force:
	@echo "  Nettoyage forcé..."
	@echo " Arrêt de tous les processus gcc et serveurs..."
	@-taskkill /F /IM "gcc.exe" 2>/dev/null || true
	@-taskkill /F /IM "$(SERVER).exe" 2>/dev/null || true
	@-taskkill /F /IM "$(CLIENT).exe" 2>/dev/null || true
	@-pkill -f "gcc" 2>/dev/null || true
	@-pkill -f "$(SERVER)" 2>/dev/null || true
	@-pkill -f "$(CLIENT)" 2>/dev/null || true
	@echo " Suppression des fichiers..."
	rm -f $(CLIENT) $(SERVER) server_info.txt *.o
	@echo "  Nettoyage forcé terminé"

# Test complet
test: all
	@echo "\n Test du projet UDP"
	@echo "1. Dans un terminal: make run-server ADDR=127.0.0.1 PORT=8080"
	@echo "2. Dans un autre terminal: make run-client"

# Aide
help:
	@echo " Commandes disponibles:"
	@echo "  make all          - Compile client et serveur"
	@echo "  make client       - Compile seulement le client"
	@echo "  make server       - Compile seulement le serveur"
	@echo "  make run-server ADDR=... PORT=... - Lance le serveur"
	@echo "  make run-client   - Lance le client"
	@echo "  make stop-server  - Arrête le serveur sans nettoyer"
	@echo "  make clean        - Nettoie les fichiers + arrête le serveur"
	@echo "  make clean-force  - Nettoyage agressif (si clean ne suffit pas)"
	@echo "  make status       - Vérifie l'état du serveur"
	@echo "  make test         - Aide pour tester"
	@echo ""
	@echo " Exemples:"
	@echo "  make run-server ADDR=127.0.0.1 PORT=5000"
	@echo "  make run-server ADDR=0.0.0.0 PORT=8080"
	@echo "  make clean       # Arrête le serveur et nettoie"

# Indique que ces règles ne correspondent pas à des fichiers
.PHONY: all clean clean-force client server run-server run-client stop-server status test help