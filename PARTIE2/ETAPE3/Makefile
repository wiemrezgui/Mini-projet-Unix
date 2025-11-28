CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Fichiers objets communs
COMMON_OBJ = common.o
ADDR ?= 127.0.0.1
PORT ?= 8080
# Tous les exécutables
ALL = serverTCP server_date server_files server_content server_stats clientTCP

.PHONY: all clean run stop

all: $(ALL)

# Compilation de common.o
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

# serverTCP (serveur principal)
serverTCP: serverTCP.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) serverTCP.c $(COMMON_OBJ) -o serverTCP $(LDFLAGS)

# Serveur spécialisé: Date
server_date: server_date.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) server_date.c $(COMMON_OBJ) -o server_date $(LDFLAGS)

# Serveur spécialisé: Fichiers
server_files: server_files.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) server_files.c $(COMMON_OBJ) -o server_files $(LDFLAGS)

# Serveur spécialisé: Contenu
server_content: server_content.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) server_content.c $(COMMON_OBJ) -o server_content $(LDFLAGS)

# Serveur spécialisé: Stats
server_stats: server_stats.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) server_stats.c $(COMMON_OBJ) -o server_stats $(LDFLAGS)

# clientTCP
clientTCP: clientTCP.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) clientTCP.c $(COMMON_OBJ) -o clientTCP $(LDFLAGS)

# Lancer tous les serveurs en arrière-plan
run: all
	@echo "=== Démarrage des serveurs spécialisés ==="
	./server_date &
	./server_files &
	./server_content &
	./server_stats &
	@sleep 1
	@echo "=== Démarrage du serverTCP ==="
	./serverTCP $(ADDR) $(PORT)

# Arrêter tous les serveurs
stop:
	@echo "Arrêt de tous les serveurs..."
	@pkill -f serveur_date || true
	@pkill -f serveur_fichiers || true
	@pkill -f serveur_contenu || true
	@pkill -f server_stats || true
	@pkill -f proxy || true
	@rm -f server_info.txt
	@echo "✓ Tous les serveurs arrêtés"
# Nettoyage
clean:
	rm -f $(ALL) *.o users.txt server_info.txt
	@echo "Nettoyage effectué"
help:
	@echo "Usage du Makefile:"
	@echo "  make              - Compiler tous les programmes"
	@echo "  make run          - Lancer tous les serveurs (défaut: 127.0.0.1:8080)"
	@echo "  make run ADDR=<ip> PORT=<port> - Lancer avec IP/port personnalisés"
	@echo "  make stop         - Arrêter tous les serveurs"
	@echo "  make clean        - Nettoyer les fichiers générés"
	@echo ""
	@echo "Exemples:"
	@echo "  make run ADDR=192.168.1.10 PORT=9000"
	@echo "  make run PORT=7777"	