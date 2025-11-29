CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Fichiers objets communs
COMMON_OBJ = common.o
ADDR ?= 127.0.0.1
PORT ?= 8080

# Ports des serveurs spécialisés
PORT_DATE = 8081
PORT_FILES = 8082
PORT_CONTENT = 8083
PORT_STATS = 8084

# Tous les exécutables
ALL = serverTCP server_date server_files server_content server_stats clientTCP

.PHONY: all clean clean-ports stop run help

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

# Nettoyer les ports - Tue les processus utilisant les ports
clean-ports:
	@echo " Nettoyage des ports des serveurs..."
	@echo "Arrêt des processus sur les ports $(PORT), $(PORT_DATE), $(PORT_FILES), $(PORT_CONTENT), $(PORT_STATS)..."

	@-if [ "$$(uname -o 2>/dev/null)" = "Cygwin" ]; then \
		for p in $(PORT) $(PORT_DATE) $(PORT_FILES) $(PORT_CONTENT) $(PORT_STATS); do \
			PIDS=$$(netstat -ano 2>/dev/null | grep ":$$p " | awk '{print $$5}' | sort -u); \
			if [ -n "$$PIDS" ]; then \
				for pid in $$PIDS; do \
					taskkill /F /PID $$pid > /dev/null 2>&1 && echo "%s PID $$pid arrêté" || echo " Échec PID $$pid"; \
				done; \
			else \
				echo "%s Port $$p libre"; \
			fi; \
		done; \
	fi
	
	@sleep 2
	@echo " Nettoyage des ports terminé"
# Arrêter tous les serveurs (alias de clean-ports)
stop: clean-ports
	@echo "🛑 Tous les serveurs ont été arrêtés"

# Nettoyage complet - Arrête les serveurs ET supprime les fichiers
clean: clean-ports
	@echo " Suppression des fichiers exécutables et temporaires..."
	rm -f $(ALL) *.o users.txt server_info.txt
	@echo "%s Nettoyage complet effectué"

# Lancer tous les serveurs en arrière-plan
run: all clean-ports
	@echo " Démarrage de l'écosystème multiserveur..."
	@echo " Configuration:"
	@echo "   - Proxy principal: $(ADDR):$(PORT)"
	@echo "   - Service Date: port $(PORT_DATE)"
	@echo "   - Service Fichiers: port $(PORT_FILES)"
	@echo "   - Service Contenu: port $(PORT_CONTENT)"
	@echo "   - Service Stats: port $(PORT_STATS)"
	@echo ""
	
	@echo "=== Démarrage des serveurs spécialisés ==="
	@echo " Lancement serveur Date sur port $(PORT_DATE)..."
	@./server_date &
	@sleep 1
	
	@echo "%s Lancement serveur Fichiers sur port $(PORT_FILES)..."
	@./server_files  &
	@sleep 1
	
	@echo " Lancement serveur Contenu sur port $(PORT_CONTENT)..."
	@./server_content  &
	@sleep 1
	
	@echo " Lancement serveur Stats sur port $(PORT_STATS)..."
	@./server_stats  &
	@sleep 2
	
	@echo "%s Tous les serveurs spécialisés démarrés en arrière-plan"
	@echo " Logs disponibles dans: server_*.log"
	
	# Vérification que les serveurs sont bien lancés
	@echo ""
	@echo " Vérification des serveurs..."
	@for p in $(PORT_DATE) $(PORT_FILES) $(PORT_CONTENT) $(PORT_STATS); do \
		if netstat -ano 2>/dev/null | grep ":$$p " > /dev/null; then \
			echo "%s Serveur sur port $$p: EN ÉCOUTE"; \
		else \
			echo " Serveur sur port $$p: NON DÉMARRÉ"; \
		fi; \
	done
	@echo ""
	
	@echo "=== Démarrage du serveur principal (Proxy) ==="
	@echo " Serveur principal sur $(ADDR):$(PORT)..."
	./serverTCP $(ADDR) $(PORT)

# Lancer seulement le client
client: clientTCP
	@echo "👤 Lancement du client..."
	./clientTCP

# Aide
help:
	@echo "Usage du Makefile - Système Multiserveur:"
	@echo ""
	@echo "  make all           - Compiler tous les programmes"
	@echo "  make run           - Lancer tous les serveurs (défaut: 127.0.0.1:8080)"
	@echo "  make run ADDR=<ip> PORT=<port> - Lancer avec IP/port personnalisés"
	@echo "  make client        - Lancer seulement le client"
	@echo "  make stop          - Arrêter tous les serveurs"
	@echo "  make clean         - Nettoyer COMPLET (arrête serveurs + supprime fichiers)"
	@echo "  make clean-ports   - Arrêter les serveurs sans supprimer les fichiers"
	@echo "  make status        - Vérifier l'état des serveurs"
	@echo ""
	@echo "Ports utilisés:"
	@echo "  - Proxy: $(PORT) (configurable)"
	@echo "  - Date: $(PORT_DATE)"
	@echo "  - Fichiers: $(PORT_FILES)"
	@echo "  - Contenu: $(PORT_CONTENT)"
	@echo "  - Stats: $(PORT_STATS)"
	@echo ""
	@echo "Exemples:"
	@echo "  make run ADDR=192.168.1.10 PORT=9000"
	@echo "  make run PORT=7777"
	@echo "  make clean          # Arrête tout et nettoie"
	@echo "  make stop           # Arrête les serveurs sans nettoyer"
	@echo "  make status         # Vérifie si les serveurs tournent"