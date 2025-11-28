#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int load_users(User users[]) {
    FILE *file = fopen(USERS_FILE, "r");
    if (file == NULL) {
        printf("[AUTH] Fichier %s non trouvé.\n", USERS_FILE);
        return 0;
    }
    
    int count = 0;
    char line[BUFFER_SIZE];
    
    while (fgets(line, sizeof(line), file) && count < MAX_USERS) {
        line[strcspn(line, "\n")] = 0;
        
        char *token = strtok(line, ":");
        if (token != NULL) {
            strcpy(users[count].username, token);
            
            token = strtok(NULL, ":");
            if (token != NULL) {
                strcpy(users[count].password, token);
                count++;
            }
        }
    }
    
    fclose(file);
    printf("[AUTH] %d utilisateurs chargés\n", count);
    return count;
}

int authenticate_user(User users[], int user_count, char *username, char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}