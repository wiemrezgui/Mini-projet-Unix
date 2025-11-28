#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char elapsed_str[BUFFER_SIZE];
    
    // Recevoir le temps écoulé depuis le proxy
    int bytes = recv(client_socket, elapsed_str, BUFFER_SIZE - 1, 0);
    elapsed_str[bytes] = '\0';
    
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
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_STATS);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("[SERVICE STATS]  Prêt\n");
    
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