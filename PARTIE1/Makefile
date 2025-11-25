# Makefile pour compiler le client et le server UDP
# Utilisation: make        (compile tout)
#              make clean   (nettoie les fichiers compilés)
#              make client  (compile seulement le client)
#              make server (compile seulement le server)

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g

# Noms des exécutables
CLIENT = clientUDP
SERVER = serverUDP

# Règle par défaut: compile tout
all: $(CLIENT) $(SERVER)
	@echo "Compilation terminée avec succès!"
	@echo "Utilisation:"
	@echo "  Serveur: ./$(SERVER) <adresse> <port>"
	@echo "  Client:  ./$(CLIENT)"

# Compilation du client
$(CLIENT): clientUDP.c
	$(CC) $(CFLAGS) -o $(CLIENT) clientUDP.c
	@echo "Client compilé: $(CLIENT)"

# Compilation du serveur
$(SERVER): serverUDP.c
	$(CC) $(CFLAGS) -o $(SERVER) serverUDP.c
	@echo "Serveur compilé: $(SERVER)"

# Règles individuelles
client: $(CLIENT)

server: $(SERVER)

# Nettoyage des fichiers compilés
clean:
	rm -f $(CLIENT) $(SERVER) server_info.txt
	@echo "Fichiers nettoyés"

# Règle pour exécuter les tests
test: all
	@echo "\n=== Test du projet UDP ==="
	@echo "1. Démarrez le serveur dans un terminal: ./$(SERVER) 127.0.0.1 8080"
	@echo "2. Dans un autre terminal, lancez le client: ./$(CLIENT)"

# Indique que ces règles ne correspondent pas à des fichiers
.PHONY: all clean client server test