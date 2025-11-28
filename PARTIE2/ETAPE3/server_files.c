#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "common.h"

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char dir_path[BUFFER_SIZE];
    DIR *dir;
    struct dirent *entry;
    
    // Recevoir le nom du répertoire
    int bytes = recv(client_socket, dir_path, BUFFER_SIZE - 1, 0);
    dir_path[bytes] = '\0';
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        snprintf(buffer, BUFFER_SIZE, " Erreur: Répertoire '%.50s' non trouvé", dir_path);
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== FICHIERS DU RÉPERTOIRE '%.50s' ===\n", dir_path);
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
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_FICHIERS);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("[SERVICE FICHIERS] ✓ Prêt\n");
    
    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (fork() == 0) {
            close(server_socket);
            handle_request(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    
    return 0;
}