#define _POSIX_C_SOURCE 200809L

#include "server_cmd.h"
#include "message_queue.h"
#include "shared_memory.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>

static volatile sig_atomic_t check_notifications_daemon = 0;

static void server_request_daemon(mqd_t *mq, const char *msg){
    message_queue_send(*mq, msg);
}

static void server_receive_data_shm(double *data_cpu_usage, double *data_mem_usage) {
    shared_memory_read(data_cpu_usage, data_mem_usage);
}

char *pid_convert_str(pid_t *pid_server) {
    char *buffer = malloc(16);
    if (buffer) {
        sprintf(buffer, "%d", *pid_server);
    }
    return buffer; // caller nhớ free
}


static void signal_handler(int signo) {
    if (signo == SIGUSR1) {
        // printf("Daemon reported data write conplete\n");
        check_notifications_daemon = 1;
    }
}

static void setup_signal() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));        // Khởi tạo toàn bộ struct về 0
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;           // tự restart syscall bị gián đoạn

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction");
    }
}

/*----------------------------------------------------------------------------------------*/

void display_commands_server(void) {
    int i = 0;
    int SIZE = sizeof(commands) / sizeof(commands[0]);
    printf("\n======================== SERVER COMMANDS ======================\n");
    for (; i < SIZE ;) {
        printf("%d. %-23s : %s\n", i + 1, commands[i].command, commands[i].description);
        i++;
    }   
    printf("===============================================================\n\n");
}

void cmd_list(void) {
    print_clients();
}

void cmd_terminate(int id_client) {
    kill_client(id_client);
}

void cmd_message(void) {
    read_log();
}

void cmd_sysinfo(pid_t *pid_server, mqd_t *mq) {
    char *pid_str = pid_convert_str(pid_server);
    setup_signal();
    double data_cpu_usage;
    double data_mem_usage;
    server_request_daemon(mq, pid_str);
    
    printf("Getting data...\n");
 
    // Chờ signal từ daemon
    while (!check_notifications_daemon);

    // Sau khi có signal thì đọc dữ liệu
    server_receive_data_shm(&data_cpu_usage, &data_mem_usage);      // Đọc từ shared memory

    // In ra màn hình
    printf("CPU usage = %.2f%%\n", data_cpu_usage);
    printf("RAM usage = %.2f%%\n", data_mem_usage); 

    check_notifications_daemon = 0;
}


void cmd_add_client(int sock_fd, const char *ip, int port, connection_status_t status) {
    add_client(sock_fd, ip, port, status);
}