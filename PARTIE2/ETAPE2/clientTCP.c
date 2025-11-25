#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char server_address[100];
    int server_port;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char directory[BUFFER_SIZE];
    int authenticated = 0;

    printf("=== CLIENT TCP - SERVICE LISTE FICHIERS ===\n");

    // Lecture des infos du serveur
    FILE *info_file = fopen(SERVER_INFO_FILE, "r");
    if (info_file == NULL) {
        perror("[CLIENT] Fichier info serveur introuvable");
        printf("[CLIENT] Lancez le serveur d'abord!\n");
        exit(1);
    }

    if (fscanf(info_file, "%s %d", server_address, &server_port) != 2) {
        fprintf(stderr, "[CLIENT] Format fichier invalide\n");
        fclose(info_file);
        exit(1);
    }
    fclose(info_file);

    printf("[CLIENT] Serveur: %s:%d\n", server_address, server_port);

    // Création socket client
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("[CLIENT] Erreur création socket");
        exit(1);
    }

    // Connexion au serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("[CLIENT] Adresse IP invalide");
        close(client_socket);
        exit(1);
    }

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[CLIENT] Erreur connexion serveur");
        close(client_socket);
        exit(1);
    }

    printf("[CLIENT] Connecté au serveur\n\n");

    // AUTHENTIFICATION
    printf("=== AUTHENTIFICATION ===\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    // Envoyer identifiants
    snprintf(buffer, BUFFER_SIZE, "AUTH:%s:%s", username, password);
    send(client_socket, buffer, strlen(buffer), 0);
    
    // Recevoir réponse
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("\n[CLIENT] Réponse: %s\n", buffer);
        
        if (strstr(buffer, "AUTH_SUCCESS") != NULL) {
            authenticated = 1;
        } else {
            printf("[CLIENT] Authentification échouée\n");
            close(client_socket);
            return 1;
        }
    }

    // SERVICE PRINCIPAL - LISTE FICHIERS
    if (authenticated) {
        printf("\n=== SERVICE LISTE FICHIERS ===\n");
        printf("Tapez 'QUIT' pour quitter\n\n");
        
        while (1) {
            printf("Nom du répertoire à lister: ");
            scanf("%s", directory);
            
            if (strcmp(directory, "QUIT") == 0) {
                send(client_socket, "QUIT", 4, 0);
                break;
            }
            
            // Envoyer le nom du répertoire
            send(client_socket, directory, strlen(directory), 0);
            
            // Recevoir la liste
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("\n=== LISTE DES FICHIERS ===\n%s\n", buffer);
            } else {
                printf("[CLIENT] Serveur déconnecté\n");
                break;
            }
        }
    }
    
    printf("[CLIENT] Déconnexion...\n");
    close(client_socket);
    printf("[CLIENT] Au revoir!\n");
    return 0;
}