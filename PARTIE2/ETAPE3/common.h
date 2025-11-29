#ifndef COMMON_H
#define COMMON_H

#define BUFFER_SIZE 1024
#define SERVER_INFO_FILE "server_info.txt"
#define USERS_FILE "users.txt"
#define SERVERS_PIDS_FILE "servers-pids"
#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50

// Ports des serveurs spécialisés
#define PORT_PRINCIPAL 8080 
#define PORT_DATE 8081
#define PORT_FICHIERS 8082
#define PORT_CONTENU 8083
#define PORT_STATS 8084

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} User;

int load_users(User users[]);
int authenticate_user(User users[], int user_count, char *username, char *password);

extern void register_my_pid(void);
extern void unregister_my_pid(void);
extern void kill_all_servers(void);
extern void signal_handler(int sig);

#endif