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
	@echo "🧹 Nettoyage des ports des serveurs..."
	@echo "🔫 Arrêt des processus sur les ports $(PORT), $(PORT_DATE), $(PORT_FILES), $(PORT_CONTENT), $(PORT_STATS)..."
	
	# Méthode 1: Utiliser fuser (Linux)
	@-for p in $(PORT) $(PORT_DATE) $(PORT_FILES) $(PORT_CONTENT) $(PORT_STATS); do \
		if command -v fuser >/dev/null 2>&1; then \
			fuser -k $${p}/tcp 2>/dev/null || true; \
		fi; \
	done
	
	# Méthode 2: Utiliser pkill avec les noms des exécutables
	@-pkill -f "serverTCP" || true
	@-pkill -f "server_date" || true
	@-pkill -f "server_files" || true
	@-pkill -f "server_content" || true
	@-pkill -f "server_stats" || true
	
	# Méthode 3: Utiliser killall (Linux)
	@-if command -v killall >/dev/null 2>&1; then \
		killall serverTCP 2>/dev/null || true; \
		killall server_date 2>/dev/null || true; \
		killall server_files 2>/dev/null || true; \
		killall server_content 2>/dev/null || true; \
		killall server_stats 2>/dev/null || true; \
	fi
	
	# Méthode 4: Cygwin/Windows - utiliser taskkill
	@-if [ "$$(uname -o 2>/dev/null)" = "Cygwin" ]; then \
		for p in $(PORT) $(PORT_DATE) $(PORT_FILES) $(PORT_CONTENT) $(PORT_STATS); do \
			netstat -ano | grep ":$$p " | awk '{print $$5}' | while read pid; do \
				taskkill /F /PID $$pid 2>/dev/null || true; \
			done; \
		done; \
	fi
	
	@sleep 2
	@echo " Ports nettoyés et processus arrêtés"

# Arrêter tous les serveurs (alias de clean-ports)
stop: clean-ports
	@echo " Tous les serveurs ont été arrêtés"

# Nettoyage complet - Arrête les serveurs ET supprime les fichiers
clean: clean-ports
	@echo " Suppression des fichiers exécutables et temporaires..."
	rm -f $(ALL) *.o users.txt server_info.txt
	rm -rf logs_test_*
	@echo "Nettoyage complet effectué"

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
	@echo "Lancement serveur Date..."
	@./server_date > server_date.log 2>&1 &
	@sleep 1
	
	@echo " Lancement serveur Fichiers..."
	@./server_files > server_files.log 2>&1 &
	@sleep 1
	
	@echo " Lancement serveur Contenu..."
	@./server_content > server_content.log 2>&1 &
	@sleep 1
	
	@echo "  Lancement serveur Stats..."
	@./server_stats > server_stats.log 2>&1 &
	@sleep 2
	
	@echo " Tous les serveurs spécialisés démarrés en arrière-plan"
	@echo " Logs disponibles dans: server_*.log"
	@echo ""
	
	@echo "=== Démarrage du serveur principal (Proxy) ==="
	@echo " Serveur principal sur $(ADDR):$(PORT)..."
	./serverTCP $(ADDR) $(PORT)

# Lancer seulement le client
client: clientTCP
	@echo " Lancement du client..."
	./clientTCP

# Vérifier l'état des serveurs
status:
	@echo " État des serveurs:"
	@echo "Port $(PORT) (Proxy): $$(if lsof -Pi :$(PORT) -sTCP:LISTEN -t >/dev/null; then echo "✅ En écoute"; else echo "❌ Arrêté"; fi)"
	@echo "Port $(PORT_DATE) (Date): $$(if lsof -Pi :$(PORT_DATE) -sTCP:LISTEN -t >/dev/null; then echo "✅ En écoute"; else echo "❌ Arrêté"; fi)"
	@echo "Port $(PORT_FILES) (Fichiers): $$(if lsof -Pi :$(PORT_FILES) -sTCP:LISTEN -t >/dev/null; then echo "✅ En écoute"; else echo "❌ Arrêté"; fi)"
	@echo "Port $(PORT_CONTENT) (Contenu): $$(if lsof -Pi :$(PORT_CONTENT) -sTCP:LISTEN -t >/dev/null; then echo "✅ En écoute"; else echo "❌ Arrêté"; fi)"
	@echo "Port $(PORT_STATS) (Stats): $$(if lsof -Pi :$(PORT_STATS) -sTCP:LISTEN -t >/dev/null; then echo "✅ En écoute"; else echo "❌ Arrêté"; fi)"
	
	@echo ""
	@echo " Processus en cours:"
	@pgrep -fl "server_" || echo "Aucun processus serveur trouvé"

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