#include "server_utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

clientInfo_t clients[MAX_CLIENTS];
int client_count = 0;    

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *log_path = "/home/haidoan2098/Workspace/MiniProject_SysMonChat/log/server_messages.log";  


void add_client(int sock_fd, const char *ip, int port, connection_status_t status) {
    pthread_mutex_lock(&client_mutex);

    // Thêm mới nếu chưa đầy
    if (client_count < MAX_CLIENTS) {
        clients[client_count].id = client_count;
        clients[client_count].sock_fd = sock_fd;
        strncpy(clients[client_count].ip, ip, INET_ADDRSTRLEN - 1);
        clients[client_count].ip[INET_ADDRSTRLEN - 1] = '\0';
        clients[client_count].port = port;   
        clients[client_count].status = status;
        client_count++;
    } 
    else {
        // Tái sử dụng slot đã ngắt kết nối
        bool replaced = false;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].status == DISCONNECTED) {
                clients[i].id = client_count;
                clients[i].sock_fd = sock_fd;
                strncpy(clients[i].ip, ip, INET_ADDRSTRLEN - 1);
                clients[i].ip[INET_ADDRSTRLEN - 1] = '\0';
                clients[i].port = port;   
                clients[i].status = status;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            printf("Client list is full!\n");
        }
    }

    pthread_mutex_unlock(&client_mutex);
}

void print_clients() {
    pthread_mutex_lock(&client_mutex);

    printf("\n============= Current Clients List =============\n");
    for (int i = 0; i < client_count; i++) {
        if (clients[i].status == CONNECTED) {
            printf("ID: %d  |  IP: %s  | PORT: %d\n", i, clients[i].ip, clients[i].port);
        }
    }
    printf("================================================\n\n");

    pthread_mutex_unlock(&client_mutex);
}

void kill_client(int id_client) {
    pthread_mutex_lock(&client_mutex);

    if (id_client >= 0 && id_client < MAX_CLIENTS && clients[id_client].status == CONNECTED) {
        send(clients[id_client].sock_fd, "__KILL__", strlen("__KILL__"), 0);     // Clinet nhận được cái này thì cút    
        close(clients[id_client].sock_fd);
        clients[id_client].status = DISCONNECTED;

        printf("\nTerminated Client ID %d\n", id_client);
    } else {
        printf("Invalid client ID or already disconnected\n");
    }

    pthread_mutex_unlock(&client_mutex);
}

void write_log(const char *msg) {
    char buffer[BUF_SIZE];      
    int len = snprintf(buffer, sizeof(buffer), "%s", msg);

    if (len < 0) {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    int fd = open(log_path,  O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open log");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    ssize_t ret = write(fd, buffer, len);
    fsync(fd);      
    if (ret < 0) {
        perror("write log");
    }

    close(fd);
    pthread_mutex_unlock(&log_mutex);
}

void read_log() {
    char buffer[BUF_SIZE];
    ssize_t n;

    pthread_mutex_lock(&log_mutex);

    int fd = open(log_path, O_RDONLY);
    if (fd < 0) {
        perror("open log");
        pthread_mutex_unlock(&log_mutex);
        return; 
    }

    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';       
        printf("%s", buffer);
    }

    if (n < 0) {
        perror("read log file");
    }

    close(fd);
    pthread_mutex_unlock(&log_mutex);
}