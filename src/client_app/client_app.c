#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE];

    // 1. Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 3. Kết nối đến server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%d\n", server_ip, server_port);

    // 4. Vòng lặp gửi & nhận
    while (1) {
        printf("You: ");
        if (!fgets(buf, BUF_SIZE, stdin)) break;

        // Xóa newline
        buf[strcspn(buf, "\n")] = 0;

        if (strcmp(buf, "quit") == 0) break;

        send(sockfd, buf, strlen(buf), 0);

        int len = recv(sockfd, buf, BUF_SIZE - 1, 0);
        if (len <= 0) {
            printf("Server disconnected.\n");
            break;    
        }
        buf[len] = '\0';
        if (strcmp(buf, "__KILL__") == 0) {
            printf("You have been kicked by the server.\n");
            break;
        }
        printf("Server: %s\n", buf);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
