#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char server_address[100];
    int server_port;
    char choice;

    printf("=== CLIENT TCP MONOSERVICE ===\n");

    // Lecture des informations du serveur depuis le fichier
    FILE *info_file = fopen(SERVER_INFO_FILE, "r");
    if (info_file == NULL)
    {
        perror(" [ CLIENT ] Impossible de lire le fichier d'information du serveur");
        printf(" [ CLIENT ] Assurez-vous que le serveur est démarré\n");
        exit(1);
    }

    if (fscanf(info_file, "%s %d", server_address, &server_port) != 2)
    {
        fprintf(stderr, " [ CLIENT ] Format du fichier d'information invalide\n");
        fclose(info_file);
        exit(1);
    }
    fclose(info_file);

    printf(" [ CLIENT ] Serveur trouvé: %s:%d\n", server_address, server_port);

    // CRÉATION DU SOCKET CLIENT
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror(" [ CLIENT ] Erreur création socket");
        exit(1);
    }

    // CONFIGURATION ET CONNEXION AU SERVEUR
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror(" [ CLIENT ] Adresse IP invalide");
        close(client_socket);
        exit(1);
    }

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror(" [ CLIENT ] Erreur connexion au serveur");
        close(client_socket);
        exit(1);
    }

    printf(" [ CLIENT ] Connecté au serveur\n");

    do
    {
        // Envoi de la demande de service
        char *request = "DATE";
        printf(" [ CLIENT ] Envoi de la demande: %s\n", request);
        send(client_socket, request, strlen(request), 0);

        // Réception de la réponse
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            printf("\n=== RÉPONSE DU SERVEUR ===\n");
            printf("%s\n", buffer);
        }
        else if (bytes_received == 0)
        {
            printf(" [ CLIENT ] Serveur déconnecté\n");
            break;
        }
        else
        {
            perror("Erreur réception");
            break;
        }

        // Demander si l'utilisateur veut continuer
        printf("\n[ CLIENT ] Voulez-vous envoyer une autre requête ? (o/n): ");
        scanf(" %c", &choice);
        while (getchar() != '\n')
            ;

    } while (choice == 'o' || choice == 'O');

    // Envoyer une commande QUIT au serveur
    printf(" [ CLIENT ] Déconnexion du serveur...\n");
    send(client_socket, "QUIT", 4, 0);
    close(client_socket);
    printf(" [ CLIENT ] Connexion fermée\n");
    printf(" [ CLIENT ] Client terminé. Au revoir!\n");
    return 0;
}