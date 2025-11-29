#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "common.h"

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char filepath[BUFFER_SIZE];
    
    // Recevoir la requête
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        close(client_socket);
        return;
    }
    buffer[bytes] = '\0';
    
    // Nettoyer le chemin reçu
    strcpy(filepath, buffer);
    
    // Supprimer espaces, newlines, guillemets
    filepath[strcspn(filepath, "\r\n")] = 0;
    
    // Supprimer les guillemets simples si présents
    char *start = filepath;
    if (*start == '\'') start++;
    char *end = start + strlen(start) - 1;
    if (*end == '\'') *end = '\0';
    
    strcpy(filepath, start);
    
    printf("[SERVICE CONTENU] Demande pour: '%s'\n", filepath);
    fflush(stdout);
    
    // Ouvrir le fichier
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Erreur: fichier non trouvé ou inaccessible\n");
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, "=== CONTENU DU FICHIER ===\n");
    int offset = strlen(buffer);
    
    char line[256];
    int line_count = 0;
    while (fgets(line, sizeof(line), file) && offset < BUFFER_SIZE - 300) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "%s", line);
        offset = strlen(buffer);
        line_count++;
    }
    
    if (line_count == 0) {
        snprintf(buffer + offset, BUFFER_SIZE - offset, "(Fichier vide)\n");
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
    
    signal(SIGCHLD, sigchld_handler);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[SERVICE CONTENU] Erreur socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_CONTENU);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVICE CONTENU] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, 5) < 0) {
        perror("[SERVICE CONTENU] Erreur listen");
        close(server_socket);
        exit(1);
    }
    
    printf("[SERVICE CONTENU]  Prêt\n");
    
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