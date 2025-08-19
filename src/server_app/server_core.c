#include "server_core.h"
#include "message_queue.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/mman.h>


// Hàm xử lý client (chạy trong thread)
void *client_handler(void *arg) {
    clientInfo_t *new_client = (clientInfo_t *)arg;

    char buffer[BUF_SIZE];
    char log_buffer[BUF_SIZE + 64];

    // Log client kết nối
    snprintf(log_buffer, sizeof(log_buffer), "[New Client %d | %s:%d] CONNECTED\n", new_client->id, new_client->ip, new_client->port);
    write_log(log_buffer);

    while (1) {
        ssize_t recv_len = recv(new_client->sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (recv_len <= 0) {
            // Client ngắt kết nối
            snprintf(log_buffer, sizeof(log_buffer), "[Client %d | %s:%d] DISCONNECTED\n", new_client->id, new_client->ip, new_client->port);
            write_log(log_buffer);

            new_client->status = DISCONNECTED;
            close(new_client->sock_fd);
            break;
        }

        buffer[recv_len] = '\0';

        // Log tin nhắn client
        snprintf(log_buffer, sizeof(log_buffer), "[Client %d | %s:%d] %s\n", new_client->id, new_client->ip, new_client->port, buffer);
        write_log(log_buffer);

        // Echo lại đúng nội dung client gửi
        send(new_client->sock_fd, buffer, recv_len, 0);
    }

    return NULL;
}

void *command_handler(void *arg) {
    mqd_t *mq = (mqd_t *)arg;
    char cmd[128];      

    while (1) {
        printf("server> ");

        fflush(stdout);
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            printf("\nInput error or EOF, exiting command handler.\n");
            break;
        }

        // Xóa kí tự xuống dòng
        cmd[strcspn(cmd, "\n")] = '\0';

        if (strcmp(cmd, "list") == 0) {
            cmd_list();
        } 
        else if (strcmp(cmd, "message") == 0) {
            cmd_message();
        }
        else if (strncmp(cmd, "terminate", 9) == 0) {
            int id = atoi(cmd + 10);   // con trỏ tới được chỗ ID (terminated <ID>)
            cmd_terminate(id);
        }
        else if (strcmp(cmd, "sysinfo") == 0) {
            pid_t pid_server = getpid();
            cmd_sysinfo(&pid_server, mq);
        }
        else if (strcmp(cmd, "exit") == 0) {
            shm_unlink("/my_shared_memory");
            mq_unlink("/my_message_queue");
            break;
        } 
        else {
            printf("Unknown command: %s\n", cmd);
        }
    }

    return NULL;
}

// -----------------------------------------
int server_init(int port) {
    int server_fd;   
    struct sockaddr_in server_addr;

    // 1. Tạo socket file descriptor 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Gắn địa chỉ (IP + Port) vào socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);     // Đổi port đang Little Edian -> Big Edian (Network byte order)

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);     
    }

    // 3. Chuyển sang chế độ lắng nghe
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening at Port %d ...\n", port);

    return server_fd;
}

void server_run(int server_fd) {
    struct sockaddr_in client_addr; 
    socklen_t client_len = sizeof(client_addr);
    mqd_t mq = message_queue_create();

    pthread_t cmd_thread;
    pthread_create(&cmd_thread, NULL, command_handler, &mq);
    pthread_detach(cmd_thread);
        

    while (1) {
        int client_sockfd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0) {
            perror("accept");
            continue;
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));  // Đổi IP (Network byte order) -> Little edian
        int port = ntohs(client_addr.sin_port);      // Đổi port (Network byte order) -> Little edian

        cmd_add_client(client_sockfd, ip_str, port, CONNECTED); 
        
        // Tạo thread để xử lý client 
        pthread_t client_thread;  
        pthread_create(&client_thread, NULL, client_handler, &clients[client_count - 1]);
        pthread_detach(client_thread);
    }
}
