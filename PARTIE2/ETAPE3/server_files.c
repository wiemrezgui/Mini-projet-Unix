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
    
    // Extraire le chemin du répertoire
    // Format attendu: "LIST /chemin" ou juste "/chemin"
    if (strncmp(buffer, "LIST ", 5) == 0) {
        strcpy(dir_path, buffer + 5);
    } else {
        strcpy(dir_path, buffer);
    }
    
    // Supprimer les espaces/newlines
    dir_path[strcspn(dir_path, "\r\n")] = 0;
    
    printf("[SERVICE FICHIERS] Demande pour: '%s'\n", dir_path);
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Erreur: Répertoire '%s' non trouvé", dir_path);
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== FICHIERS DU RÉPERTOIRE '%s' ===\n", dir_path);
    int offset = strlen(buffer);
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        snprintf(buffer + offset, BUFFER_SIZE - offset, "- %s\n", entry->d_name);
        offset = strlen(buffer);
        
        if (offset >= BUFFER_SIZE - 100) {
            strcat(buffer, "... (liste tronquée)");
            break;
        }
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVICE FICHIERS] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, 5) < 0) {
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
    
    return 0;
}
