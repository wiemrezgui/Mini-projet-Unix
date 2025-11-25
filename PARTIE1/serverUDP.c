#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int socketfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int port, n, i;
    
    // Vérification des arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <adresse> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[2]);
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    
    // Création du socket UDP
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        perror(" [ SERVER ] Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }
    
    printf(" [ SERVER ] Socket créé avec succès\n");
    
    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Conversion de l'adresse IP
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror(" [ SERVER ] Erreur d'adresse IP invalide");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    // Liaison du socket à l'adresse et au port
    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror(" [ SERVER ] Erreur lors du bind");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    printf(" [ SERVER ] Serveur en écoute sur %s:%s...\n\n", argv[1], argv[2]);
    
    // Enregistrement des informations du serveur dans un fichier
    FILE *info_file = fopen(SERVER_INFO_FILE, "w");
    if (info_file == NULL) {
        perror(" [ SERVER ] Erreur lors de la création du fichier d'information");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    fprintf(info_file, "%s %s", argv[1], argv[2]);
    fclose(info_file);
    
    printf("[ SERVER ] Informations du serveur enregistrées dans %s\n", SERVER_INFO_FILE);
    
    // Boucle principale du serveur
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        client_len = sizeof(client_addr);
        
        // Réception du message du client
        int recv_len = recvfrom(socketfd, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *)&client_addr, &client_len);
        
        if (recv_len < 0) {
            perror("Erreur lors de la réception");
            continue;
        }
        
        buffer[recv_len] = '\0';
        
        // Affichage des informations du client
        printf(" [ SERVER ] Message reçu de %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        // Conversion du nombre reçu
        n = atoi(buffer); // Convertit la chaîne en nombre
        printf(" [ SERVER ] Nombre reçu: %d\n", n);
        
        // Vérification que n est valide
        if (n < 1 || n > NMAX) {
            sprintf(buffer, " [ SERVER ] Erreur: n doit être entre 1 et %d", NMAX);
        } else {
            // Génération de n nombres aléatoires
            memset(buffer, 0, BUFFER_SIZE);
            
            for (i = 0; i < n; i++) {
                int nombre_aleatoire = rand() % NMAX + 1; // Nombres entre 1 et NMAX
                char temp[50];
                sprintf(temp, "%d", nombre_aleatoire);
                
                if (i == 0) {
                    strcpy(buffer, temp);
                } else {
                    strcat(buffer, " ");
                    strcat(buffer, temp);
                }
            }
        }
        
        // Envoi de la réponse au client
        if (sendto(socketfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&client_addr, client_len) < 0) {
            perror(" [ SERVER ] Erreur lors de l'envoi");
        } else {
            printf(" [ SERVER ] Réponse envoyée au client: %s\n", buffer);
        }
        
        printf("------------------------------------\n\n");
    }
    
    close(socketfd);
    // Supprimer le fichier d'information à la fermeture
    remove(SERVER_INFO_FILE);
    return 0;
}