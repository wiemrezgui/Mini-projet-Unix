#!/bin/bash

# Script de test pour clients multiples avec un seul log détaillé par client
# Usage: ./test_multiclient.sh [nombre_de_clients]

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Nombre de clients (défaut: 5)
NUM_CLIENTS=${1:-5}

echo -e "${BLUE}=== TEST MULTICLIENT - $NUM_CLIENTS CLIENTS ===${NC}"

# Vérifier que le serveur est en cours d'exécution
if [ ! -f "server_info.txt" ]; then
    echo -e "${RED} Le serveur ne semble pas être démarré${NC}"
    echo "Lancez d'abord: ./serveurTCP 127.0.0.1 8082"
    exit 1
fi

# Vérifier que le fichier users.txt existe
if [ ! -f "users.txt" ]; then
    echo -e "${RED} Fichier users.txt non trouvé${NC}"
    exit 1
fi

echo -e "${GREEN} Utilisateurs détectés:${NC}"
cat users.txt

# Créer un dossier de logs avec timestamp
LOG_DIR="logs_test_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$LOG_DIR"
echo -e "${PURPLE} Dossier de logs: $LOG_DIR${NC}"

# Fichier temporaire pour les PIDs
PIDS_FILE="/tmp/client_pids_$$"

# Fonction de nettoyage
cleanup() {
    echo -e "\n${YELLOW}🛑 Arrêt des clients...${NC}"
    if [ -f "$PIDS_FILE" ]; then
        while read pid; do
            kill $pid 2>/dev/null
        done < "$PIDS_FILE"
        rm -f "$PIDS_FILE"
    fi
    # Nettoyer les fichiers temporaires
    rm -f /tmp/client_input_*.txt
    echo -e "${GREEN} Nettoyage terminé${NC}"
    echo -e "${PURPLE} Les logs sont conservés dans: $LOG_DIR${NC}"
}

# Capturer Ctrl+C
trap cleanup EXIT INT TERM

# Lire les utilisateurs depuis le fichier
USERS=()
PASSWORDS=()
i=0

while IFS=: read -r user pass; do
    USERS[$i]="$user"
    PASSWORDS[$i]="$pass"
    ((i++))
done < users.txt

TOTAL_USERS=$i

if [ $TOTAL_USERS -eq 0 ]; then
    echo -e "${RED} Aucun utilisateur trouvé dans users.txt${NC}"
    exit 1
fi

echo -e "${GREEN} $TOTAL_USERS utilisateurs chargés${NC}"

# Lancer les clients en arrière-plan
echo -e "${YELLOW} Lancement de $NUM_CLIENTS clients...${NC}"

for ((i=0; i<NUM_CLIENTS && i<TOTAL_USERS; i++)); do
    USERNAME="${USERS[$i]}"
    PASSWORD="${PASSWORDS[$i]}"
    CLIENT_NUM=$((i+1))
    
    echo -e "${BLUE}Client $CLIENT_NUM${NC} - Utilisateur: $USERNAME"
    
    # Créer un fichier d'entrée automatique pour chaque client
    cat > /tmp/client_input_$i.txt << EOF
$USERNAME
$PASSWORD
.
..
../..
PARTIE1
PARTIE2
QUIT
EOF

    # Un seul fichier log détaillé par client
    CLIENT_LOG="$LOG_DIR/client_${CLIENT_NUM}_${USERNAME}.log"
    
    # En-tête du log
    echo "=== LOG DÉTAILLÉ CLIENT $CLIENT_NUM ===" > "$CLIENT_LOG"
    echo "Utilisateur: $USERNAME" >> "$CLIENT_LOG"
    echo "Date de début: $(date)" >> "$CLIENT_LOG"
    echo "======================================" >> "$CLIENT_LOG"
    echo "" >> "$CLIENT_LOG"
    
    # Lancer le client avec entrée automatique et redirection vers le log
    ./clientTCP < /tmp/client_input_$i.txt 2>&1 | while IFS= read -r line; do
        echo "[$(date '+%H:%M:%S')] $line" >> "$CLIENT_LOG"
    done &
    
    CLIENT_PID=$!
    echo $CLIENT_PID >> "$PIDS_FILE"
    
    echo -e "${GREEN} Client $CLIENT_NUM démarré (PID: $CLIENT_PID) - $USERNAME${NC}"
    echo -e "${CYAN}   Log: $CLIENT_LOG${NC}"
    
    # Petit délai entre les lancements
    sleep 2
done

echo -e "\n${YELLOW} Tous les clients sont lancés...${NC}"
echo -e "${PURPLE} Tous les logs sont dans: $LOG_DIR${NC}"
echo -e "${YELLOW} Surveillance en cours (Ctrl+C pour arrêter)${NC}"


# Surveiller les logs en temps réel
while true; do
    clear
    echo -e "${BLUE}=== ÉTAT DES CLIENTS ===${NC}"
    echo -e "${PURPLE} Dossier de logs: $LOG_DIR${NC}"
    echo ""
    
    ALL_FINISHED=true
    for ((i=0; i<NUM_CLIENTS && i<TOTAL_USERS; i++)); do
        USERNAME="${USERS[$i]}"
        CLIENT_NUM=$((i+1))
        CLIENT_LOG="$LOG_DIR/client_${CLIENT_NUM}_${USERNAME}.log"
        
        if [ -f "$CLIENT_LOG" ]; then
            # Vérifier l'état du client
            if tail -5 "$CLIENT_LOG" | grep -q "Au revoir"; then
                STATUS=" Terminé"
                COLOR=$GREEN
            elif tail -10 "$CLIENT_LOG" | grep -q "RÉPONSE DU SERVEUR"; then
                STATUS=" Service actif"
                COLOR=$BLUE
            elif tail -10 "$CLIENT_LOG" | grep -q "AUTH_SUCCESS"; then
                STATUS=" Authentifié"
                COLOR=$GREEN
            elif tail -10 "$CLIENT_LOG" | grep -q "AUTH_FAILED"; then
                STATUS=" Auth échouée"
                COLOR=$RED
            elif tail -10 "$CLIENT_LOG" | grep -q "Connecté"; then
                STATUS=" En connexion"
                COLOR=$YELLOW
            else
                STATUS=" Démarrage..."
                COLOR=$YELLOW
                ALL_FINISHED=false
            fi
            
            echo -e "Client $CLIENT_NUM ($USERNAME): ${COLOR}${STATUS}${NC}"
        else
            ALL_FINISHED=false
        fi
    done
      
    # Si tous les clients ont terminé, sortir
    if [ "$ALL_FINISHED" = true ]; then
        echo -e "\n${GREEN}Tous les clients ont terminé!${NC}"
        break
    fi    
    sleep 2
done


echo -e "\n${GREEN} Test terminé!${NC}"
echo -e "${PURPLE} Les logs complets sont disponibles dans: $LOG_DIR/${NC}"
