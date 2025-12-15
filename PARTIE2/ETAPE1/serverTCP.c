#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "common.h"

void get_current_datetime(char* buffer) {
    time_t rawtime;          // Stocke le temps en secondes
    struct tm* timeinfo;     // Structure pour temps décomposé
    
    time(&rawtime);          // Récupère le temps actuel système
    timeinfo = localtime(&rawtime);  // Convertit en temps local (fuseau horaire)
    
    strftime(buffer, BUFFER_SIZE, 
             "Date: %A %d %B %Y\nHeure: %H:%M:%S", timeinfo);
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int port;
    
    // Vérification des arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <adresse> <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[2]); // conversion du port en entier
    
    printf("=== SERVEUR TCP MONOSERVICE ===\n");
    printf(" [ SERVER ] Service disponible: Date/Heure du serveur\n");
    
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[ SERVER ] Erreur création socket");
        exit(1);
    }
    printf("[ SERVER ] Socket serveur créé\n");
    
    // Configuration adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Conversion de l'adresse IP
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("[ SERVER ] Adresse IP invalide");
        close(server_socket);
        exit(1);
    }
    
    // Liaison du socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror(" [ SERVER ] Erreur bind");
        close(server_socket);
        exit(1);
    }
    printf("[ SERVER ] Serveur lié à %s:%d\n", argv[1], port);
    
    // Enregistrement des informations dans le fichier
    FILE *info_file = fopen(SERVER_INFO_FILE, "w");
    if (info_file == NULL) {
        perror("[ SERVER ] Erreur création fichier info");
        close(server_socket);
        exit(1);
    }
    fprintf(info_file, "%s %d", argv[1], port);
    fclose(info_file);
    printf(" [ SERVER ] Informations enregistrées dans %s\n", SERVER_INFO_FILE);
    
    // Mise en écoute
    if (listen(server_socket, 1) < 0) {
        perror(" [ SERVER ] Erreur listen");
        close(server_socket);
        remove(SERVER_INFO_FILE);
        exit(1);
    }
    printf(" [ SERVER ] Serveur en écoute...\n\n");
    
    // Boucle principale (un client à la fois)
    while (1) {
        printf(" [ SERVER ] En attente d'un client...\n");
        
        // Acceptation d'une connexion client
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror(" [ SERVER ] Erreur accept");
            continue;
        }
        
        // Boucle pour gérer plusieurs requêtes du même client
        int client_active = 1;
        while (client_active) {
            // Réception du message du client
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf(" [ SERVER ] Message reçu: %s\n", buffer);
                
                // Vérification si le client demande la date/heure
                if (strstr(buffer, "DATE") != NULL) {
                    char datetime[BUFFER_SIZE];
                    get_current_datetime(datetime);
                    
                    // Envoi de la date/heure au client
                    send(client_socket, datetime, strlen(datetime), 0);
                    printf(" [ SERVER ] Date/heure envoyée: %s\n", datetime);
                } 
                // Gestion de la déconnexion
                else if (strstr(buffer, "QUIT") != NULL) {
                    printf(" [ SERVER ] Client a demandé à se déconnecter\n");
                    client_active = 0;
                }
                else {
                    char* message = "Service non reconnu. Envoyez 'DATE' ou 'QUIT'";
                    send(client_socket, message, strlen(message), 0);
                }
            } 
            else if (bytes_received == 0) {
                // Client s'est déconnecté proprement
                printf(" [ SERVER ] Client déconnecté\n");
                client_active = 0;
            } 
            else {
                // Erreur de réception
                perror(" [ SERVER ] Erreur réception");
                client_active = 0;
            }
        }
        
        // Fermeture de la connexion client
        close(client_socket);
        printf(" [ SERVER ] Connexion client fermée\n\n");
    }
    
    close(server_socket);
    remove(SERVER_INFO_FILE);
    return 0;
}