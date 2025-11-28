#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "common.h"

void add_users() {
    int nbr_clients;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    printf("=== CREATION DES UTILISATEURS ===\n");
    printf("Nombre de clients à ajouter: ");
    scanf("%d", &nbr_clients);
    getchar();
    
    FILE *file = fopen(USERS_FILE, "w");
    if (file == NULL) {
        perror("Erreur création fichier utilisateurs");
        exit(1);
    }
    
    for (int i = 0; i < nbr_clients; i++) {
        printf("\n--- Client %d ---\n", i + 1);
        printf("Username: ");
        fgets(username, MAX_USERNAME_LEN, stdin);
        username[strcspn(username, "\n")] = 0;
        
        printf("Mot de passe: ");
        fgets(password, MAX_PASSWORD_LEN, stdin);
        password[strcspn(password, "\n")] = 0;
        
        fprintf(file, "%s:%s\n", username, password);
        printf("Utilisateur '%s' ajouté\n", username);
    }
    
    fclose(file);
    printf("\n %d utilisateurs créés\n\n", nbr_clients);
}

void send_menu(int client_socket) {
    char menu[BUFFER_SIZE];
    snprintf(menu, BUFFER_SIZE,
             "\n╔════════════════════════════════════════╗\n"
             "║    MENU DES SERVICES DISPONIBLES      ║\n"
             "╠════════════════════════════════════════╣\n"
             "║ 1. Date et heure du serveur           ║\n"
             "║ 2. Liste des fichiers d'un répertoire ║\n"
             "║ 3. Contenu d'un fichier                ║\n"
             "║ 4. Durée de connexion                  ║\n"
             "║ 5. QUIT - Se déconnecter               ║\n"
             "╚════════════════════════════════════════╝\n"
             "Votre choix: ");
    send(client_socket, menu, strlen(menu), 0);
}

// Fonction pour relayer la requête vers un serveur spécialisé
int relay_to_server(int port, const char* request, char* response) {
    int server_socket;
    struct sockaddr_in server_addr;
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) return -1;
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        snprintf(response, BUFFER_SIZE, "Erreur: Service indisponible (port %d)", port);
        return -1;
    }
    
    // Envoyer la requête
    send(server_socket, request, strlen(request), 0);
    
    // Recevoir la réponse
    memset(response, 0, BUFFER_SIZE);
    int bytes = recv(server_socket, response, BUFFER_SIZE - 1, 0);
    response[bytes] = '\0';
    
    close(server_socket);
    return 0;
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char username[MAX_USERNAME_LEN];
    User users[MAX_USERS];
    int user_count;
    int authenticated = 0;
    time_t connection_start = time(NULL);
    
    printf("[SERVER] Connexion de %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    user_count = load_users(users);
    if (user_count == 0) {
        send(client_socket, "Erreur: Aucun utilisateur", 26, 0);
        close(client_socket);
        return;
    }
    
    // AUTHENTIFICATION
    while (!authenticated) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            close(client_socket);
            return;
        }
        buffer[bytes] = '\0';
        
        if (strncmp(buffer, "AUTH:", 5) == 0) {
            char *cred = buffer + 5;
            char *user = strtok(cred, ":");
            char *pass = strtok(NULL, ":");
            
            if (user && pass) {
                strcpy(username, user);
                if (authenticate_user(users, user_count, user, pass)) {
                    authenticated = 1;
                    send(client_socket, "AUTH_SUCCESS", 12, 0);
                    printf("[SERVER] %s authentifié\n", username);
                } else {
                    send(client_socket, "AUTH_FAILED", 11, 0);
                }
            }
        }
    }
    
    // BOUCLE DE SERVICE
    int active = 1;
    while (active) {
        send_menu(client_socket);
        
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        
        printf("[SERVER] %s choisit: %s\n", username, buffer);
        
        char response[BUFFER_SIZE];
        int target_port = 0;
        
        if (strcmp(buffer, "1") == 0) {
            target_port = PORT_DATE;
            send(client_socket, "RESULT", 6, 0);
            usleep(10000);
            
            relay_to_server(target_port, "GET_DATE", response);
            send(client_socket, response, strlen(response), 0);
            
        } else if (strcmp(buffer, "2") == 0) {
            target_port = PORT_FICHIERS;
            send(client_socket, "PROMPT", 6, 0);
            usleep(10000);
            send(client_socket, "Entrez le répertoire: ", 23, 0);
            
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                relay_to_server(target_port, buffer, response);
                send(client_socket, response, strlen(response), 0);
            }
            
        } else if (strcmp(buffer, "3") == 0) {
            target_port = PORT_CONTENU;
            send(client_socket, "PROMPT", 6, 0);
            usleep(10000);
            send(client_socket, "Entrez le fichier: ", 20, 0);
            
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                relay_to_server(target_port, buffer, response);
                send(client_socket, response, strlen(response), 0);
            }
            
        } else if (strcmp(buffer, "4") == 0) {
            target_port = PORT_STATS;
            send(client_socket, "RESULT", 6, 0);
            usleep(10000);
            
            time_t current = time(NULL);
            double elapsed = difftime(current, connection_start);
            snprintf(buffer, BUFFER_SIZE, "%.0f", elapsed);
            
            relay_to_server(target_port, buffer, response);
            send(client_socket, response, strlen(response), 0);
            
        } else if (strcmp(buffer, "5") == 0) {
            send(client_socket, "QUIT_ACK", 8, 0);
            usleep(10000);
            send(client_socket, "Au revoir!", 10, 0);
            active = 0;
        } else {
            send(client_socket, "ERROR", 5, 0);
            usleep(10000);
            send(client_socket, "Choix invalide", 15, 0);
        }
    }
    
    close(client_socket);
    printf("[SERVER] %s déconnecté\n\n", username);
}

int main(int argc, char *argv[]) {
    int proxy_socket, client_socket;
    struct sockaddr_in proxy_addr, client_addr;
    socklen_t client_len;
    char address[100];
    int port;
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <adresse> <port>\n", argv[0]);
        fprintf(stderr, "Exemple: %s 127.0.0.1 8080\n", argv[0]);
        exit(1);
    }
    
    strcpy(address, argv[1]);
    port = atoi(argv[2]);
    
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║     SERVER  - MULTISERVEUR                ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    add_users();
    
    printf("=== DÉMARRAGE DU SERVER ===\n");
    printf("Adresse: %s, Port: %d\n", address, port);
    printf("Services:\n");
    printf("  1. Date/Heure → Port %d\n", PORT_DATE);
    printf("  2. Liste fichiers → Port %d\n", PORT_FICHIERS);
    printf("  3. Contenu fichier → Port %d\n", PORT_CONTENU);
    printf("  4. Statistiques → Port %d\n\n", PORT_STATS);
    
    signal(SIGCHLD, sigchld_handler);
    
    proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket < 0) {
        perror("[SERVER] Erreur création socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(proxy_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address, &proxy_addr.sin_addr) <= 0) {
        perror("[SERVER] Adresse IP invalide");
        close(proxy_socket);
        exit(1);
    }
    
    if (bind(proxy_socket, (struct sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("[SERVER] Erreur bind");
        close(proxy_socket);
        exit(1);
    }
    
    // Enregistrer les infos du serveur dans server_info.txt
    FILE *info_file = fopen(SERVER_INFO_FILE, "w");
    if (info_file == NULL) {
        perror("[SERVER] Erreur création fichier info");
        close(proxy_socket);
        exit(1);
    }
    fprintf(info_file, "%s %d", address, port);
    fclose(info_file);
    printf("[SERVER] Infos enregistrées dans %s\n", SERVER_INFO_FILE);
    
    if (listen(proxy_socket, 5) < 0) {
        perror("[SERVER] Erreur listen");
        close(proxy_socket);
        remove(SERVER_INFO_FILE);
        exit(1);
    }
    
    printf("[SERVER] En écoute sur %s:%d\n\n", address, port);
    
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(proxy_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror("[SERVER] Erreur accept");
            continue;
        }
        
        if (fork() == 0) {
            close(proxy_socket);
            handle_client(client_socket, client_addr);
            exit(0);
        }
        close(client_socket);
    }
    
    close(proxy_socket);
    remove(SERVER_INFO_FILE);
    return 0;
}