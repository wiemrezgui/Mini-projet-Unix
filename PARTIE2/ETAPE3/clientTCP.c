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
    int authenticated = 0;

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║       CLIENT TCP                                 ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    // Lecture des infos du serveur depuis server_info.txt
    FILE *info_file = fopen(SERVER_INFO_FILE, "r");
    if (info_file == NULL) {
        perror("[CLIENT] Fichier server_info.txt introuvable");
        printf("[CLIENT]  Lancez le serveur d'abord!\n");
        exit(1);
    }

    if (fscanf(info_file, "%s %d", server_address, &server_port) != 2) {
        fprintf(stderr, "[CLIENT]  Format du fichier server_info.txt invalide\n");
        fclose(info_file);
        exit(1);
    }
    fclose(info_file);

    printf("[CLIENT] Serveur cible: %s:%d (depuis %s)\n", 
           server_address, server_port, SERVER_INFO_FILE);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("[CLIENT] Erreur socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("[CLIENT] Adresse IP invalide");
        close(client_socket);
        exit(1);
    }

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[CLIENT] Erreur connexion");
        close(client_socket);
        exit(1);
    }

    printf("[CLIENT]  Connecté au proxy\n\n");

    // AUTHENTIFICATION
    printf("═══ AUTHENTIFICATION ═══\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    getchar();
    
    snprintf(buffer, BUFFER_SIZE, "AUTH:%s:%s", username, password);
    send(client_socket, buffer, strlen(buffer), 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        if (strstr(buffer, "AUTH_SUCCESS") != NULL) {
            authenticated = 1;
            printf("\n[CLIENT]  Authentification réussie!\n");
        } else {
            printf("\n[CLIENT] ✗ %s\n", buffer);
            close(client_socket);
            return 1;
        }
    }

    // BOUCLE DE SERVICES
    if (authenticated) {
        while (1) {
            // Recevoir le menu
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                printf("[CLIENT] Proxy déconnecté\n");
                break;
            }
            
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            
            // Lire le choix
            char choice[10];
            fgets(choice, sizeof(choice), stdin);
            choice[strcspn(choice, "\n")] = 0;
            
            // Envoyer le choix
            send(client_socket, choice, strlen(choice), 0);
            
            // Si QUIT
            if (strcmp(choice, "5") == 0 || strcmp(choice, "QUIT") == 0) {
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("\n%s\n", buffer);
                }
                break;
            }
            
            // Recevoir le signal
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                printf("[CLIENT] Proxy déconnecté\n");
                break;
            }
            
            buffer[bytes_received] = '\0';
            
            // Traiter selon le signal
            if (strcmp(buffer, "PROMPT") == 0) {
                // Recevoir le prompt
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("\n%s", buffer);
                    
                    char input[BUFFER_SIZE];
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0;
                    
                    send(client_socket, input, strlen(input), 0);
                    
                    // Recevoir le résultat
                    memset(buffer, 0, BUFFER_SIZE);
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        buffer[bytes_received] = '\0';
                        printf("\n%s\n", buffer);
                    }
                }
                
            } else if (strcmp(buffer, "RESULT") == 0) {
                // Recevoir le résultat direct
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("\n%s\n", buffer);
                }
                
            } else if (strcmp(buffer, "ERROR") == 0) {
                // Recevoir l'erreur
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("\n%s\n", buffer);
                }
            }
            
            printf("\n[Appuyez sur Entrée pour continuer]\n");
            getchar();
        }
    }
    
    printf("\n[CLIENT] Déconnexion...\n");
    close(client_socket);
    printf("[CLIENT] Au revoir!\n");
    return 0;
}