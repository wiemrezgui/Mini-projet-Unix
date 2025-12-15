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

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    
    // Recevoir la requête
    recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    snprintf(buffer, BUFFER_SIZE,
             "=== DATE ET HEURE DU SERVEUR ===\n"
             "Date: %02d/%02d/%04d\n"
             "Heure: %02d:%02d:%02d\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    
    printf("[SERVICE DATE] Démarrage sur port %d\n", PORT_DATE);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    register_my_pid();
    signal(SIGCHLD, sigchld_handler);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[SERVICE DATE] Erreur socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_DATE);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SERVICE DATE] Erreur bind");
        close(server_socket);
        exit(1);
    }
    
    if (listen(server_socket, 6) < 0) {
        perror("[SERVICE DATE] Erreur listen");
        close(server_socket);
        exit(1);
    }
    
    printf("[SERVICE DATE] Prêt\n");
    
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