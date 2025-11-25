#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include "common.h"

// Fonction pour créer les utilisateurs via terminal
void add_users() {
    int nbr_clients;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    printf("=== CREATION DES UTILISATEURS ===\n");
    printf("Nombre de clients à ajouter: ");
    scanf("%d", &nbr_clients);
    getchar(); // Vider le buffer
    
    FILE *file = fopen(USERS_FILE, "w");
    if (file == NULL) {
        perror("Erreur création fichier utilisateurs");
        exit(1);
    }
    
    for (int i = 0; i < nbr_clients; i++) {
        printf("\n--- Client %d ---\n", i + 1);
        printf("Username: ");
        fgets(username, MAX_USERNAME_LEN, stdin);
        username[strcspn(username, "\n")] = 0; // Supprimer le \n
        
        printf("Mot de passe: ");
        fgets(password, MAX_PASSWORD_LEN, stdin);
        password[strcspn(password, "\n")] = 0; // Supprimer le \n
        
        // Écrire dans le fichier
        fprintf(file, "%s:%s\n", username, password);
        printf(" Utilisateur '%s' ajouté\n", username);
    }
    
    fclose(file);
    printf("\n %d utilisateurs créés dans %s\n\n", nbr_clients, USERS_FILE);
}

// Fonction pour lister les fichiers d'un répertoire
void list_directory_files(const char* dir_path, char* buffer) {
    DIR *dir;
    struct dirent *entry;
    int offset = 0;
    
    buffer[0] = '\0';
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Erreur: Répertoire '%s' non trouvé", dir_path);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== FICHIERS DU RÉPERTOIRE '%s' ===\n", dir_path);
    offset = strlen(buffer);
    
    while ((entry = readdir(dir)) != NULL) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "- %s\n", entry->d_name);
        offset = strlen(buffer);
        
        if (offset >= BUFFER_SIZE - 100) {
            strcat(buffer, "... (liste tronquée)");
            break;
        }
    }
    
    closedir(dir);
}

// Gestionnaire pour éviter les processus zombies
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Fonction pour gérer un client
void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    int authenticated = 0;
    char username[MAX_USERNAME_LEN];
    User users[MAX_USERS];
    int user_count;
    
    printf("[SERVER] Nouvelle connexion de %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    // Charger les utilisateurs
    user_count = load_users(users);
    if (user_count == 0) {
        char* error_msg = "Erreur: Aucun utilisateur dans la base";
        send(client_socket, error_msg, strlen(error_msg), 0);
        close(client_socket);
        return;
    }
    
    // PHASE D'AUTHENTIFICATION
    while (!authenticated) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            printf("[SERVER] Client déconnecté pendant auth\n");
            close(client_socket);
            return;
        }
        
        buffer[bytes_received] = '\0';
        
        if (strncmp(buffer, "AUTH:", 5) == 0) {
            char *credentials = buffer + 5;
            char *user = strtok(credentials, ":");
            char *pass = strtok(NULL, ":");
            
            if (user != NULL && pass != NULL) {
                strcpy(username, user);
                
                if (authenticate_user(users, user_count, user, pass)) {
                    authenticated = 1;
                    char* success_msg = "AUTH_SUCCESS: Authentification réussie! Envoyez un nom de répertoire";
                    send(client_socket, success_msg, strlen(success_msg), 0);
                    printf("[SERVER] ✅ %s authentifié\n", username);
                } else {
                    char* error_msg = "AUTH_FAILED: Identifiants incorrects";
                    send(client_socket, error_msg, strlen(error_msg), 0);
                    printf("[SERVER] ❌ Echec auth pour %s\n", user);
                }
            }
        } else {
            char* error_msg = "Format: AUTH:username:password";
            send(client_socket, error_msg, strlen(error_msg), 0);
        }
    }
    
    // BOUCLE DE SERVICE - LISTE DES FICHIERS
    int client_active = 1;
    while (client_active) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("[SERVER] %s demande: %s\n", username, buffer);
            
            // Vérifier si c'est QUIT
            if (strcmp(buffer, "QUIT") == 0) {
                printf("[SERVER] %s se déconnecte\n", username);
                char* bye_msg = "Au revoir!";
                send(client_socket, bye_msg, strlen(bye_msg), 0);
                client_active = 0;
            } 
            // SERVICE: Liste des fichiers du répertoire
            else {
                char file_list[BUFFER_SIZE];
                list_directory_files(buffer, file_list);
                send(client_socket, file_list, strlen(file_list), 0);
                printf("[SERVER] Liste envoyée à %s pour: %s\n", username, buffer);
            }
        } 
        else if (bytes_received == 0) {
            printf("[SERVER] %s déconnecté\n", username);
            client_active = 0;
        } 
        else {
            perror("[SERVER] Erreur réception");
            client_active = 0;
        }
    }
    
    close(client_socket);
    printf("[SERVER] Connexion %s fermée\n\n", username);
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int port;
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <adresse> <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[2]);
    
    printf("=== SERVEUR TCP MULTICLIENT - SERVICE LISTE FICHIERS ===\n");
    
    // ÉTAPE 1: Créer les utilisateurs via terminal
    add_users();
    
    // ÉTAPE 2: Démarrer le serveur
    printf("=== DÉMARRAGE DU SERVEUR ===\n");
    printf("Service: Liste des fichiers d'un répertoire\n");
    printf("Adresse: %s, Port: %d\n\n", argv[1], port);
    
    // Configurer le handler pour les processus zombies
    signal(SIGCHLD, sigchld_handler);
    
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[SERVER] Erreur création socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configuration adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("[SERVER] Adresse IP invalide");
        close(server_socket);
        exit(1);
    }
    
    // Liaison du socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVER] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    // Enregistrement des infos
    FILE *info_file = fopen(SERVER_INFO_FILE, "w");
    if (info_file == NULL) {
        perror("[SERVER] Erreur création fichier info");
        close(server_socket);
        exit(1);
    }
    fprintf(info_file, "%s %d", argv[1], port);
    fclose(info_file);
    
    // Mise en écoute
    if (listen(server_socket, 5) < 0) {
        perror("[SERVER] Erreur listen");
        close(server_socket);
        remove(SERVER_INFO_FILE);
        exit(1);
    }
    
    printf("[SERVER] Serveur en écoute sur %s:%d\n", argv[1], port);
    printf("[SERVER] Prêt à lister les répertoires...\n\n");
    
    // BOUCLE PRINCIPALE - ACCEPTE MULTIPLES CLIENTS
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror("[SERVER] Erreur accept");
            continue;
        }
        
        // Créer un processus fils pour chaque client
        pid_t pid = fork();
        
        if (pid == 0) {
            // PROCESSUS FILS
            close(server_socket);
            handle_client(client_socket, client_addr);
            exit(0);
        } else if (pid > 0) {
            // PROCESSUS PÈRE
            close(client_socket);
            printf("[SERVER] Nouveau processus (PID: %d) pour client\n", pid);
        } else {
            perror("[SERVER] Erreur fork");
            close(client_socket);
        }
    }
    
    close(server_socket);
    remove(SERVER_INFO_FILE);
    return 0;
}