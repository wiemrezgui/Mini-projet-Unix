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

void add_users()
{
    int nbr_clients;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    printf("=== CREATION DES UTILISATEURS ===\n");
    printf("Nombre de clients à ajouter: ");
    scanf("%d", &nbr_clients);
    getchar();

    FILE *file = fopen(USERS_FILE, "w");
    if (file == NULL)
    {
        perror("Erreur création fichier utilisateurs");
        exit(1);
    }

    for (int i = 0; i < nbr_clients; i++)
    {
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

void send_menu(int client_socket)
{
    char menu[BUFFER_SIZE];
    snprintf(menu, BUFFER_SIZE,
             "\n╔════════════════════════════════════════╗\n"
             "║    MENU DES SERVICES DISPONIBLES         ║\n"
             "╠══════════════════════════════════════════╣\n"
             "║ 1. Date et heure du serveur              ║\n"
             "║ 2. Liste des fichiers d'un répertoire    ║\n"
             "║ 3. Contenu d'un fichier                  ║\n"
             "║ 4. Durée de connexion                    ║\n"
             "║ 5. QUIT - Se déconnecter                 ║\n"
             "╚══════════════════════════════════════════╝\n"
             "Votre choix: ");
    send(client_socket, menu, strlen(menu), 0);
}

// Fonction pour relayer la requête vers un serveur spécialisé
int relay_to_server(int port, const char *request, char *response)
{
    int server_socket;
    struct sockaddr_in server_addr;

    // Créer socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        return -1;
    }

    // Configurer timeout de connexion
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(server_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // Configuration adresse serveur spécialisé
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connexion au serveur spécialisé
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(server_socket);
        return -1;
    }

    // Envoyer la requête
    if (send(server_socket, request, strlen(request), 0) < 0)
    {
        close(server_socket);
        return -1;
    }

    // Recevoir la réponse
    memset(response, 0, BUFFER_SIZE);
    int bytes_received = recv(server_socket, response, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0)
    {
        close(server_socket);
        return -1;
    }

    response[bytes_received] = '\0';
    close(server_socket);
    return 0;
}

void sigchld_handler(int sig)
{
    (void)sig; // Éviter l'avertissement de variable non utilisée
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
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
                    printf("[SERVER]  %s authentifié\n", username);
                } else {
                    send(client_socket, "AUTH_FAILED", 11, 0);
                    printf("[SERVER]  Échec auth pour %s\n", user);
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
        if (bytes <= 0) {
            printf("[SERVER] %s déconnecté (perte connexion)\n", username);
            break;
        }
        buffer[bytes] = '\0';
        
        printf("[SERVER] %s choisit: %s\n", username, buffer);
        
        char response[BUFFER_SIZE];
        int target_port = 0;
        char service_request[BUFFER_SIZE];
        
        if (strcmp(buffer, "1") == 0) {
            // ===== SERVICE 1: DATE ET HEURE =====
            target_port = PORT_DATE;
            printf("[SERVER] → Redirection vers service DATE (port %d)\n", target_port);
            
            if (relay_to_server(target_port, "GET_DATE", response) == 0) {
                send(client_socket, response, strlen(response), 0);
                printf("[SERVER]  Date envoyée à %s\n", username);
            } else {
                strcpy(response, "⚠️ Service DATE indisponible");
                send(client_socket, response, strlen(response), 0);
                printf("[SERVER]  Service DATE indisponible\n");
            }
            
        } else if (strcmp(buffer, "2") == 0) {
            // ===== SERVICE 2: LISTE FICHIERS =====
            target_port = PORT_FICHIERS;
            printf("[SERVER] → Redirection vers service FICHIERS (port %d)\n", target_port);
            
            // Demander le répertoire au client
            send(client_socket, "PROMPT:Entrez le répertoire:", 29, 0);
            
            // Recevoir le répertoire
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                
                // Nettoyer le chemin (supprimer espaces/newlines)
                buffer[strcspn(buffer, "\r\n")] = 0;
                
                printf("[SERVER] Répertoire demandé: '%s'\n", buffer);
                
                // Envoyer directement le chemin au serveur spécialisé (SANS guillemets)
                snprintf(service_request, BUFFER_SIZE, "%s", buffer);
                
                if (relay_to_server(target_port, service_request, response) == 0) {
                    send(client_socket, response, strlen(response), 0);
                    printf("[SERVER]  Liste fichiers envoyée à %s\n", username);
                } else {
                    strcpy(response, "⚠️ Service FICHIERS indisponible");
                    send(client_socket, response, strlen(response), 0);
                    printf("[SERVER]  Service FICHIERS indisponible\n");
                }
            } else {
                printf("[SERVER]  Erreur réception répertoire\n");
            }
            
        } else if (strcmp(buffer, "3") == 0) {
            // ===== SERVICE 3: CONTENU FICHIER =====
            target_port = PORT_CONTENU;
            printf("[SERVER] → Redirection vers service CONTENU (port %d)\n", target_port);
            
            // Demander le fichier au client
            send(client_socket, "PROMPT:Entrez le fichier:", 26, 0);
            
            // Recevoir le fichier
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                
                // Nettoyer le chemin (supprimer espaces/newlines)
                buffer[strcspn(buffer, "\r\n")] = 0;
                
                printf("[SERVER] Fichier demandé: '%s'\n", buffer);
                
                // Envoyer directement le chemin au serveur spécialisé (SANS guillemets)
                snprintf(service_request, BUFFER_SIZE, "%s", buffer);
                
                if (relay_to_server(target_port, service_request, response) == 0) {
                    send(client_socket, response, strlen(response), 0);
                    printf("[SERVER]  Contenu fichier envoyé à %s\n", username);
                } else {
                    strcpy(response, "⚠️ Service CONTENU indisponible");
                    send(client_socket, response, strlen(response), 0);
                    printf("[SERVER]  Service CONTENU indisponible\n");
                }
            } else {
                printf("[SERVER]  Erreur réception fichier\n");
            }
            
        } else if (strcmp(buffer, "4") == 0) {
            // ===== SERVICE 4: DURÉE DE CONNEXION =====
            target_port = PORT_STATS;
            printf("[SERVER] → Redirection vers service STATS (port %d)\n", target_port);
            
            // Calculer la durée de connexion
            time_t current = time(NULL);
            double elapsed = difftime(current, connection_start);
            snprintf(service_request, BUFFER_SIZE, "TIME %.0f", elapsed);
            
            printf("[SERVER] Durée connexion: %.0f secondes\n", elapsed);
            
            if (relay_to_server(target_port, service_request, response) == 0) {
                send(client_socket, response, strlen(response), 0);
                printf("[SERVER]  Stats envoyées à %s\n", username);
            } else {
                strcpy(response, "⚠️ Service STATS indisponible");
                send(client_socket, response, strlen(response), 0);
                printf("[SERVER]  Service STATS indisponible\n");
            }
            
        } else if (strcmp(buffer, "5") == 0 || strcmp(buffer, "QUIT") == 0) {
            // ===== SERVICE 5: DÉCONNEXION =====
            printf("[SERVER] %s demande la déconnexion\n", username);
            send(client_socket, "QUIT:Au revoir!", 15, 0);
            active = 0;
            
        } else {
            // ===== CHOIX INVALIDE =====
            printf("[SERVER]  Choix invalide de %s: '%s'\n", username, buffer);
            send(client_socket, "ERROR:Choix invalide (1-5)", 27, 0);
        }
        
        // Petite pause pour éviter les conflits de synchronisation
        usleep(10000); // 10ms
    }
    
    close(client_socket);
    printf("[SERVER] ═══ %s déconnecté ═══\n\n", username);
}
int main(int argc, char *argv[])
{
    int proxy_socket, client_socket;
    struct sockaddr_in proxy_addr, client_addr;
    socklen_t client_len;
    char address[100];
    int port;

    if (argc != 3)
    {
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
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    register_my_pid();
    printf("=== DÉMARRAGE DU SERVER ===\n");
    printf("Adresse: %s, Port: %d\n", address, port);
    printf("Services:\n");
    printf("  1. Date/Heure → Port %d\n", PORT_DATE);
    printf("  2. Liste fichiers → Port %d\n", PORT_FICHIERS);
    printf("  3. Contenu fichier → Port %d\n", PORT_CONTENU);
    printf("  4. Statistiques → Port %d\n\n", PORT_STATS);

    signal(SIGCHLD, sigchld_handler);

    proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket < 0)
    {
        perror("[SERVER] Erreur création socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(proxy_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &proxy_addr.sin_addr) <= 0)
    {
        perror("[SERVER] Adresse IP invalide");
        close(proxy_socket);
        exit(1);
    }

    if (bind(proxy_socket, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0)
    {
        perror("[SERVER] Erreur bind");
        close(proxy_socket);
        exit(1);
    }

    // Enregistrer les infos du serveur dans server_info.txt
    FILE *info_file = fopen(SERVER_INFO_FILE, "w");
    if (info_file == NULL)
    {
        perror("[SERVER] Erreur création fichier info");
        close(proxy_socket);
        exit(1);
    }
    fprintf(info_file, "%s %d", address, port);
    fclose(info_file);
    printf("[SERVER] Infos enregistrées dans %s\n", SERVER_INFO_FILE);

    if (listen(proxy_socket, 5) < 0)
    {
        perror("[SERVER] Erreur listen");
        close(proxy_socket);
        remove(SERVER_INFO_FILE);
        exit(1);
    }

    printf("[SERVER] En écoute sur %s:%d\n\n", address, port);

    while (1)
    {
        client_len = sizeof(client_addr);
        client_socket = accept(proxy_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0)
        {
            perror("[SERVER] Erreur accept");
            continue;
        }

        if (fork() == 0)
        {
            close(proxy_socket);
            handle_client(client_socket, client_addr);
            exit(0);
        }
        close(client_socket);
    }

    close(proxy_socket);
    remove(SERVER_INFO_FILE);
    unregister_my_pid();
    return 0;
}