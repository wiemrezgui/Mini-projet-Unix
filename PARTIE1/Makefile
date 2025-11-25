# Makefile pour compiler le client et le serveur UDP
# Utilisation: make        (compile tout)
#              make clean   (nettoie les fichiers compilés)
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

# Exécution du serveur avec paramètres
run-server: $(SERVER)
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

# Nettoyage des fichiers compilés
clean:
	rm -f $(CLIENT) $(SERVER) server_info.txt
	@echo " Fichiers nettoyés"

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
	@echo "  make test         - Aide pour tester"
	@echo "  make clean        - Nettoie les fichiers compilés"
	@echo ""
	@echo " Exemples:"
	@echo "  make run-server ADDR=127.0.0.1 PORT=5000"
	@echo "  make run-server ADDR=0.0.0.0 PORT=8080"
	@echo "  make run-client"