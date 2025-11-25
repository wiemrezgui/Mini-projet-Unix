# Makefile pour TCP
CC = gcc
CFLAGS = -Wall -Wextra -g

CLIENT_TCP = clientTCP
SERVER_TCP = serverTCP

# Règle par défaut
all: $(CLIENT_TCP) $(SERVER_TCP)
	@echo "Compilation TCP terminée!"
	@echo ""
	@echo "Utilisation:"
	@echo "  Server: ./$(SERVER_TCP) <adresse> <port>"
	@echo "  Client:  ./$(CLIENT_TCP)"
	@echo ""
	@echo " Commandes rapides:"
	@echo "  make run-server-tcp ADDR=127.0.0.1 PORT=8080"
	@echo "  make run-client-tcp"

# Compilation TCP
$(CLIENT_TCP): clientTCP.c common.h
	$(CC) $(CFLAGS) -o $(CLIENT_TCP) clientTCP.c

$(SERVER_TCP): serverTCP.c common.h
	$(CC) $(CFLAGS) -o $(SERVER_TCP) serverTCP.c

# Règles individuelles
client-tcp: $(CLIENT_TCP)
server-tcp: $(SERVER_TCP)

# Exécution TCP
run-server-tcp: $(SERVER_TCP)
	@if [ -z "$(ADDR)" ] || [ -z "$(PORT)" ]; then \
		echo " Usage: make run-server-tcp ADDR=127.0.0.1 PORT=8080"; \
		exit 1; \
	fi
	@echo " Lancement du serveur TCP sur $(ADDR):$(PORT)..."
	./$(SERVER_TCP) $(ADDR) $(PORT)

run-client-tcp: $(CLIENT_TCP)
	@echo " Lancement du client TCP..."
	./$(CLIENT_TCP)

# Nettoyage
clean:
	rm -f $(CLIENT_TCP) $(SERVER_TCP) server_tcp_info.txt
	@echo " Fichiers TCP nettoyés"

# Aide
help:
	@echo " Commandes TCP disponibles:"
	@echo "  make all              - Compile client et serveur TCP"
	@echo "  make client-tcp       - Compile seulement le client TCP"
	@echo "  make server-tcp       - Compile seulement le serveur TCP"
	@echo "  make run-server-tcp ADDR=... PORT=... - Lance le serveur TCP"
	@echo "  make run-client-tcp   - Lance le client TCP"
	@echo "  make clean            - Nettoie les fichiers"

.PHONY: all clean client-tcp server-tcp run-server-tcp run-client-tcp help