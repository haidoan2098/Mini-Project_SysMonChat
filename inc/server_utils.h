#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include <netinet/in.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 2048

typedef enum {
    DISCONNECTED = 0,
    CONNECTED = 1
} connection_status_t;

// Thông tin của 1 client 
typedef struct {
    int id;
    int sock_fd;
    char ip[INET_ADDRSTRLEN];   // IPv4 string
    int port;  
    connection_status_t status;  
} clientInfo_t;

// Mảng chứa thông tin của tất cả client 
extern clientInfo_t clients[MAX_CLIENTS];
// Đếm số lượng client 
extern int client_count;    


void add_client(int sock_fd, const char *ip, int port, connection_status_t status);
void print_clients(void);
void kill_client(int id_client);

/**
 * @brief Write the status and messages of all clients sent to the server to the log.
 */
void write_log(const char *msg);

/**
 * @brief Read the status and messages of all clients sent to the server in log to console.
 */
void read_log(void);

#endif