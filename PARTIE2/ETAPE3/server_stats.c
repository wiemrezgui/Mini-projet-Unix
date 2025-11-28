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
    char elapsed_str[BUFFER_SIZE];
    
    // Recevoir la requête
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        close(client_socket);
        return;
    }
    buffer[bytes] = '\0';
    
    // Extraire le temps
    // Format attendu: "TIME 123" ou juste "123"
    if (strncmp(buffer, "TIME ", 5) == 0) {
        strcpy(elapsed_str, buffer + 5);
    } else {
        strcpy(elapsed_str, buffer);
    }
    
    double elapsed = atof(elapsed_str);
    int hours = (int)elapsed / 3600;
    int minutes = ((int)elapsed % 3600) / 60;
    int seconds = (int)elapsed % 60;
    
    snprintf(buffer, BUFFER_SIZE,
             "=== DURÉE DE CONNEXION ===\n"
             "Temps écoulé: %02d:%02d:%02d\n"
             "(soit %.0f secondes)\n",
             hours, minutes, seconds, elapsed);
    
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    
    printf("[SERVICE STATS] Démarrage sur port %d\n", PORT_STATS);
    
    signal(SIGCHLD, sigchld_handler);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[SERVICE STATS] Erreur socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_STATS);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVICE STATS] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, 5) < 0) {
        perror("[SERVICE STATS] Erreur listen");
        close(server_socket);
        exit(1);
    }
    
    printf("[SERVICE STATS]  Prêt\n");
    
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