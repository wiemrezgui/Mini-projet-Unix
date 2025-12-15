#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include "common.h"

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char dir_path[BUFFER_SIZE];
    DIR *dir;
    struct dirent *entry;
    
    // Recevoir la requête
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        close(client_socket);
        return;
    }
    buffer[bytes] = '\0';
    
    // Nettoyer le chemin reçu
    strcpy(dir_path, buffer);
    
    // Supprimer espaces, newlines, guillemets
    dir_path[strcspn(dir_path, "\r\n")] = 0;
    
    // Supprimer les guillemets simples si présents
    char *start = dir_path;
    if (*start == '\'') start++;
    char *end = start + strlen(start) - 1;
    if (*end == '\'') *end = '\0';
    
    // Si le chemin est vide, utiliser le répertoire courant
    if (strlen(start) == 0 || strcmp(start, ".") == 0) {
        strcpy(dir_path, ".");
    } else {
        strcpy(dir_path, start);
    }
    
    printf("[SERVICE FICHIERS] Demande pour: '%s'\n", dir_path);
    fflush(stdout);
    
    // Ouvrir le répertoire
    dir = opendir(dir_path);
    if (dir == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Erreur: Répertoire non trouvé ou inaccessible\n");
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== FICHIERS DU RÉPERTOIRE ===\n");
    int offset = strlen(buffer);
    
    int file_count = 0;
    while ((entry = readdir(dir)) != NULL) {
          if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        snprintf(buffer + offset, BUFFER_SIZE - offset, "- %s\n", entry->d_name);
        offset = strlen(buffer);
        file_count++;

    }
    
    if (file_count == 0) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "(Répertoire vide)\n");
    }
    
    closedir(dir);
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    
    printf("[SERVICE FICHIERS] Démarrage sur port %d\n", PORT_FICHIERS);
    
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    register_my_pid();
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[SERVICE FICHIERS] Erreur socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_FICHIERS);
    server_addr.sin_addr.s_addr = INADDR_ANY; // ecouter sur toutes les interfaces 
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVICE FICHIERS] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, 6) < 0) {
        perror("[SERVICE FICHIERS] Erreur listen");
        close(server_socket);
        exit(1);
    }
    
    printf("[SERVICE FICHIERS]  Prêt\n");
    
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) continue;
        
        if (fork() == 0) {
            close(server_socket);
            handle_request(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    unregister_my_pid();
    return 0;
}