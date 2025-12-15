CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

ADDR ?= 127.0.0.1
PORT ?= 8090

# Ports des serveurs spécialisés
PORT_DATE = 8091
PORT_FILES = 8092
PORT_CONTENT = 8093
PORT_STATS = 8094

# Tous les exécutables
ALL = serverTCP server_date server_files server_content server_stats clientTCP

.PHONY: all clean stop run help status

all: help $(ALL)

# serverTCP (serveur principal) - AJOUT de pid_manager.c
serverTCP: serverTCP.c common.c pid_manager.c common.h
	$(CC) $(CFLAGS) serverTCP.c common.c pid_manager.c -o serverTCP $(LDFLAGS)

# Serveur spécialisé: Date - AJOUT de pid_manager.c
server_date: server_date.c common.c pid_manager.c common.h
	$(CC) $(CFLAGS) server_date.c common.c pid_manager.c -o server_date $(LDFLAGS)

# Serveur spécialisé: Fichiers - AJOUT de pid_manager.c
server_files: server_files.c common.c pid_manager.c common.h
	$(CC) $(CFLAGS) server_files.c common.c pid_manager.c -o server_files $(LDFLAGS)

# Serveur spécialisé: Contenu - AJOUT de pid_manager.c
server_content: server_content.c common.c pid_manager.c common.h
	$(CC) $(CFLAGS) server_content.c common.c pid_manager.c -o server_content $(LDFLAGS)

# Serveur spécialisé: Stats - AJOUT de pid_manager.c
server_stats: server_stats.c common.c pid_manager.c common.h
	$(CC) $(CFLAGS) server_stats.c common.c pid_manager.c -o server_stats $(LDFLAGS)

# clientTCP (pas besoin de pid_manager)
clientTCP: clientTCP.c common.c common.h
	$(CC) $(CFLAGS) clientTCP.c common.c -o clientTCP $(LDFLAGS)

# Cible pour arrêter les serveurs via le fichier PIDs
stop:
	@if [ -f "servers-pids" ]; then \
		echo "Arrêt de tous les serveurs enregistrés..."; \
		while read pid; do \
			if kill $$pid 2>/dev/null; then \
				echo " Signal envoyé au PID $$pid"; \
			else \
				echo " Impossible d'arrêter le PID $$pid"; \
			fi; \
		done < servers-pids; \
		rm -f servers-pids; \
		echo "Tous les serveurs ont reçu le signal d'arrêt"; \
	else \
		echo "Aucun serveur enregistré (fichier servers-pids introuvable)"; \
	fi

# Nettoyage complet
clean: stop
	@echo "Suppression des fichiers exécutables et temporaires..."
	rm -f $(ALL) *.o users.txt server_info.txt servers-pids temp-pids
	@echo "Nettoyage complet effectué"

# Lancer tous les serveurs en arrière-plan
run: all 
	@echo " Démarrage de l'écosystème multiserveur..."
	@echo ""
	@echo " CONFIGURATION :"
	@echo "   Serveur principal : $(ADDR):$(PORT)"
	@echo "   Service Date    : port $(PORT_DATE)"
	@echo "   Service Fichiers: port $(PORT_FILES)"
	@echo "   Service Contenu : port $(PORT_CONTENT)"
	@echo "   Service Stats   : port $(PORT_STATS)"
	@echo ""
	
	@echo " INSTRUCTIONS IMPORTANTES :"
	@echo "=========================================="
	@echo "Le système va démarrer TOUS les serveurs en arrière-plan."
	@echo ""
	@echo "Pour tester avec des clients, ouvrez de NOUVEAUX TERMINAUX et :"
	@echo "   1. Lancer un client : ./clientTCP"
	@echo "   2. Pour plusieurs clients : répétez dans d'autres terminaux"
	@echo ""
	@echo "Pour arrêter TOUT le système :"
	@echo "   Dans CE terminal : Ctrl+C (serveur principal)"
	@echo "   Puis : make stop"
	@echo "=========================================="
	@echo ""
	
	# Nettoyer l'ancien fichier PIDs
	@rm -f servers-pids
	
	@echo "=== Démarrage des serveurs spécialisés ==="
	@echo " Lancement serveur Date sur port $(PORT_DATE)..."
	@./server_date &
	@sleep 1
	
	@echo " Lancement serveur Fichiers sur port $(PORT_FILES)..."
	@./server_files &
	@sleep 1
	
	@echo " Lancement serveur Contenu sur port $(PORT_CONTENT)..."
	@./server_content &
	@sleep 1
	
	@echo " Lancement serveur Stats sur port $(PORT_STATS)..."
	@./server_stats &
	@sleep 2
	
	@echo " Tous les serveurs spécialisés démarrés en arrière-plan"
	
	# Vérification des ports
	@echo ""
	@echo " Vérification des serveurs..."
	@for p in $(PORT_DATE) $(PORT_FILES) $(PORT_CONTENT) $(PORT_STATS); do \
		if netstat -tuln 2>/dev/null | grep ":$$p " > /dev/null || \
		   (netstat -ano 2>/dev/null | grep ":$$p " > /dev/null); then \
			echo "   Port $$p:  EN ÉCOUTE"; \
		else \
			echo "   Port $$p:  NON DÉMARRÉ"; \
		fi; \
	done
	
	@echo ""
	@echo "=== Démarrage du serveur principal (Main server) ==="
	@echo " Serveur principal sur $(ADDR):$(PORT)..."
	@echo ""
	@echo " RAPPEL :"
	@echo "   Pour tester : ouvrir un autre terminal et 'make client'"
	@echo "   Pour arrêter : Ctrl+C puis 'make stop'"
	@echo "=========================================="
	@echo ""
	./serverTCP $(ADDR) $(PORT)

# Lancer seulement le client
client: clientTCP
	@echo "Lancement du client..."
	./clientTCP

# Aide
help:
	@echo "Usage du Makefile - Système Multiserveur:"
	@echo ""
	@echo "  make all           - Compiler tous les programmes"
	@echo "  make run           - Lancer tous les serveurs (défaut: 127.0.0.1:8080)"
	@echo "  make run ADDR=<ip> PORT=<port> - Lancer avec IP/port personnalisés"
	@echo "  make client        - Lancer seulement le client"
	@echo "  make stop          - Arrêter tous les serveurs (via fichier PIDs)"
	@echo "  make clean         - Nettoyer COMPLET (arrête serveurs + supprime fichiers)"
	@echo ""
	@echo "Ports utilisés:"
	@echo "  - Proxy: $(PORT) (configurable)"
	@echo "  - Date: $(PORT_DATE)"
	@echo "  - Fichiers: $(PORT_FILES)"
	@echo "  - Contenu: $(PORT_CONTENT)"
	@echo "  - Stats: $(PORT_STATS)"