#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

void register_my_pid() {
    // Utiliser un verrou fichier pour éviter les conflits
    FILE *file = fopen(SERVERS_PIDS_FILE, "a");
    if (file == NULL) {
        perror("Erreur ouverture fichier PIDs");
        return;
    }
    
    // S'assurer que le PID est bien écrit
    fprintf(file, "%d\n", getpid());
    fflush(file); // Forcer l'écriture
    fclose(file);
    
    printf(" [PID %d] Serveur démarré et enregistré dans %s\n", getpid(), SERVERS_PIDS_FILE);
}

void unregister_my_pid() {
     FILE *file = fopen(SERVERS_PIDS_FILE, "r");
    if (file == NULL) {
        printf("[PID %d] Fichier PIDs déjà supprimé\n", getpid());
        return;
    }
    
    FILE *temp = fopen("temp-pids", "w");
    if (temp == NULL) {
        fclose(file);
        return;
    }
    
    pid_t current_pid;
    int found = 0;
    while (fscanf(file, "%d", &current_pid) == 1) {
        if (current_pid != getpid()) {
            fprintf(temp, "%d\n", current_pid);
        } else {
            found = 1;
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (found) {
        remove(SERVERS_PIDS_FILE);
        rename("temp-pids", SERVERS_PIDS_FILE);
        printf("[PID %d] Retiré du fichier PIDs\n", getpid());
    } else {
        remove("temp-pids");
    }
}

void kill_all_servers() {
    FILE *file = fopen(SERVERS_PIDS_FILE, "r");
    if (file == NULL) {
        printf("Aucun serveur enregistré\n");
        return;
    }
    
    pid_t pid;
    printf("\n=== ARRÊT DE TOUS LES SERVEURS ===\n");
    
    while (fscanf(file, "%d", &pid) == 1) {
        if (pid != getpid()) {
            kill(pid, SIGTERM);
            printf(" Signal envoyé au PID %d\n", pid);
        }
    }
    
    fclose(file);
    
    // SUPPRESSION du fichier
    if (remove(SERVERS_PIDS_FILE) == 0) {
        printf(" Fichier servers-pids supprimé\n");
    } else {
        printf(" Impossible de supprimer servers-pids\n");
    }
    
    printf("Tous les serveurs arrêtés\n");
}

// Gestionnaire de signal
void signal_handler(int sig) {
    (void)sig;
    kill_all_servers();    
    unregister_my_pid();
    exit(0);
}