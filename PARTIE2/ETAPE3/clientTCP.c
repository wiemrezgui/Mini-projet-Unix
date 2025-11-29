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

    // Lecture des infos du serveur
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

    printf("[CLIENT] Serveur cible: %s:%d\n", server_address, server_port);

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
    getchar(); // Vider le buffer
    
    snprintf(buffer, BUFFER_SIZE, "AUTH:%s:%s", username, password);
    send(client_socket, buffer, strlen(buffer), 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        if (strcmp(buffer, "AUTH_SUCCESS") == 0) {
            authenticated = 1;
            printf("\n[CLIENT]  Authentification réussie!\n");
        } else {
            printf("\n[CLIENT]  Authentification échouée\n");
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
            printf("> ");
            fgets(choice, sizeof(choice), stdin);
            choice[strcspn(choice, "\n")] = 0;
            
            // Envoyer le choix
            send(client_socket, choice, strlen(choice), 0);
            
            // Recevoir la réponse
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                printf("[CLIENT] Proxy déconnecté\n");
                break;
            }
            
            buffer[bytes_received] = '\0';
            
            // Analyser le type de réponse
            if (strncmp(buffer, "PROMPT:", 7) == 0) {
                // C'est une demande d'entrée utilisateur
                char *prompt_text = buffer + 7;
                printf("\n%s ", prompt_text);
                
                char user_input[BUFFER_SIZE];
                fgets(user_input, sizeof(user_input), stdin);
                user_input[strcspn(user_input, "\n")] = 0;
                
                // Envoyer l'entrée utilisateur
                send(client_socket, user_input, strlen(user_input), 0);
                
                // Recevoir le résultat final
                memset(buffer, 0, BUFFER_SIZE);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("\n%s\n", buffer);
                }
                
            } else if (strncmp(buffer, "QUIT:", 5) == 0) {
                // C'est un message de déconnexion
                char *quit_msg = buffer + 5;
                printf("\n%s\n", quit_msg);
                break;
                
            } else if (strncmp(buffer, "ERROR:", 6) == 0) {
                // C'est une erreur
                char *error_msg = buffer + 6;
                printf("\n %s\n", error_msg);
                
            } else {
                // C'est un résultat direct
                printf("\n%s\n", buffer);
            }
            
            // Pause pour la lisibilité
            printf("\n[Appuyez sur Entrée pour continuer...]");
            getchar();
        }
    }
    
    printf("\n[CLIENT] Déconnexion...\n");
    close(client_socket);
    printf("[CLIENT] Au revoir!\n");
    return 0;
}