#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char filepath[BUFFER_SIZE];
    
    // Recevoir le chemin du fichier
    int bytes = recv(client_socket, filepath, BUFFER_SIZE - 1, 0);
    filepath[bytes] = '\0';
    
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Erreur: Fichier '%.50s' non trouvé", filepath);
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== CONTENU DU FICHIER '%.50s' ===\n", filepath);
    int offset = strlen(buffer);
    
    char line[256];
    while (fgets(line, sizeof(line), file) && offset < BUFFER_SIZE - 300) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "%s", line);
        offset = strlen(buffer);
    }
    
    fclose(file);
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    
    printf("[SERVICE CONTENU] Démarrage sur port %d\n", PORT_CONTENU);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_CONTENU);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("[SERVICE CONTENU]  Prêt\n");
    
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
