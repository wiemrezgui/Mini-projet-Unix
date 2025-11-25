#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serveur_addr;
    char buffer[BUFFER_SIZE];
    int n;
    socklen_t len;
    char choice;
    char server_address[100];
    int server_port;
    
    // Lecture des informations du serveur depuis le fichier
    FILE *info_file = fopen(SERVER_INFO_FILE, "r");
    if (info_file == NULL) {
        perror("[ CLIENT ] Erreur: impossible de lire le fichier d'information du serveur");
        printf(" [ CLIENT ] Assurez-vous que le serveur est démarré\n");
        exit(EXIT_FAILURE);
    }
    
    if (fscanf(info_file, "%s %d", server_address, &server_port) != 2) {
        fprintf(stderr, "[ CLIENT ] Erreur: format du fichier d'information invalide\n");
        fclose(info_file);
        exit(EXIT_FAILURE);
    }
    fclose(info_file);
    
    printf(" [ CLIENT ] Serveur trouvé: %s:%d\n", server_address, server_port);
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    
    // Création du socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror(" [ CLIENT ] Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }
    
    printf("[ CLIENT ] Socket créé avec succès\n");
    
    // Configuration de l'adresse du serveur
    memset(&serveur_addr, 0, sizeof(serveur_addr));
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_port = htons(server_port);
    
    // Conversion de l'adresse IP
    if (inet_pton(AF_INET, server_address, &serveur_addr.sin_addr) <= 0) {
        perror(" [ CLIENT ]Erreur d'adresse IP invalide");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    len = sizeof(serveur_addr);
    
    // Boucle pour permettre plusieurs requêtes
    do {
        // Génération d'un nombre aléatoire n entre 1 et NMAX
        n = rand() % NMAX + 1;
        printf("\n═══════════════════════════════════════════\n");
        printf(" [ CLIENT ] Requête vers %s:%d\n", server_address, server_port);
        printf("[ CLIENT ] Envoi du nombre n=%d au serveur...\n", n);
        
        // Envoi du nombre n au serveur
        sprintf(buffer, "%d", n);
        
        if (sendto(sockfd, buffer, strlen(buffer), 0, 
                   (struct sockaddr *)&serveur_addr, len) < 0) {
            perror("[ CLIENT ] Erreur lors de l'envoi");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        
        printf(" [ CLIENT ] Nombre envoyé au serveur: %d\n", n);
        
        // Réception de la réponse du serveur
        memset(buffer, 0, BUFFER_SIZE);
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                                (struct sockaddr *)&serveur_addr, &len);
        
        if (recv_len < 0) {
            perror("[ CLIENT ] Erreur lors de la réception");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        
        // Affichage de la réponse
        buffer[recv_len] = '\0';
        printf("\n[ CLIENT ]Réponse reçue du serveur:\n");
        printf("%s\n", buffer);
        
        // Demander si l'utilisateur veut continuer
        printf("\n[ CLIENT ] Voulez-vous envoyer une autre requête ? (o/n): ");
        scanf(" %c", &choice);
        
        // Vider le buffer d'entrée
        while (getchar() != '\n');
        
    } while (choice == 'o' || choice == 'O');
    
    // Fermeture du socket
    close(sockfd);
    printf("\n[ CLIENT ] Connexion fermée\n");
    
    return 0;
}