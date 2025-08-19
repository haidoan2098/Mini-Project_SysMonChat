#include "daemon.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

typedef struct {
    float cpu_usage;        
    float memory_usage;
} sysinfo_t;

const char *path_cpu_stat = "/proc/stat";
const char *path_memory_info = "/proc/meminfo";


// Hàm đọc cpu từ /proc/stat
static bool read_cpu_stat(cpu_stat_t *stat) {
    int fd = open(path_cpu_stat, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return false;  
    }

    char buffer[2048];  
    ssize_t len;
    
    while ((len = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[len] = '\0'; 

    }

    if (len < 0) {
        perror("read");
        close(fd);
        return false; 
    }

    sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu",
        &stat->user, &stat->nice, &stat->system, &stat->idle,
        &stat->iowait, &stat->irq, &stat->softirq);

    close(fd);
    return true;
}

// Hàm tính CPU usage % giữa 2 lần lấy 
static double compute_cpu_usage(cpu_stat_t *prev, cpu_stat_t *curr) {
    unsigned long long prev_busy = prev->user + prev->nice + prev->system + prev->irq + prev->softirq;
    unsigned long long curr_busy = curr->user + curr->nice + curr->system + curr->irq + curr->softirq;

    unsigned long long prev_total = prev_busy + prev->idle + prev->iowait;
    unsigned long long curr_total = curr_busy + curr->idle + curr->iowait;

    unsigned long long delta_busy = curr_busy - prev_busy;
    unsigned long long delta_total = curr_total - prev_total;

    if (delta_total == 0) return 0.0;

    return (double)delta_busy / delta_total * 100.0;
}

// Hàm đọc mem từ /proc/meminfo và lấy data
static bool read_mem_info(mem_info_t *info) {
    char buffer[2048];
    ssize_t len;

    int fd = open(path_memory_info, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return false;
    }

    len = read(fd, buffer, sizeof(buffer) - 1);

    if (len < 0) {
        perror("read");
        close(fd);
        return false;
    }

    buffer[len] = '\0';         

    close(fd);

    char *line = strtok(buffer, "\n");      // Lấy đoạn đầu tiên \n trong buffer

    while (line != NULL) {
        sscanf(line, "MemTotal:        %llu kB", &info->memtotal);
        sscanf(line, "MemAvailable:    %llu kB", &info->memavailable);
        line = strtok(NULL, "\n");
    }

    return true;
}

// Hàm chuyển pid ở dạng str thành pid_t
static pid_t str_convert_pid(const char *pid_str) {
    return (pid_t)atoi(pid_str);
}

/*----------------------------------------------------------------------*/
// Lấy data CPU
double get_cpu_usage(cpu_stat_t *stat1, cpu_stat_t *stat2) {
    // Lấy lần 1
    if (read_cpu_stat(stat1) == false) return -1;

    sleep(1);   // Chờ 1s
    
    // Lấy lần 2
    if (read_cpu_stat(stat2) == false) return -1;

    return compute_cpu_usage(stat1, stat2); 
}

// Tính toán data mem chuyển về %
double get_mem_usage(mem_info_t *info) {
    if (read_mem_info(info) == false) return -1;

    return ((double)(info->memtotal - info->memavailable) / info->memtotal) * 100.0;
}

// Nhận data từ server là pid của server (Thông báo thực hiện lấy data đi)
void daemon_receive_data_mq(char *buffer) {
    message_queue_receive(buffer);
}

// Lấy data và ghi vào Shared Memory
void daemon_write_data_shm(void) {
    cpu_stat_t stat1, stat2;    
    mem_info_t info;

    double cpu = get_cpu_usage(&stat1, &stat2);
    sleep(1);
    double mem = get_mem_usage(&info);

    shared_memory_write(&cpu, &mem);
}

// Từ PID nhận được gửi signal cho Server biết
void send_signal(pid_t pid) {
    if (kill(pid, SIGUSR1) < 0) {
        perror("kill");
    }   
    printf("Send data for server\n");
}


int main(int argc, char const *argv[]) {
    char buffer[64];  // Lấy PID ở dạng str

    while (1)
    {
        daemon_receive_data_mq(buffer);     // Do bật blocking nên sẽ chờ tới khi nhận được message

        pid_t pid = str_convert_pid(buffer);    // convert thành pid_t

        daemon_write_data_shm();        // Ghi vào vùng shared memory

        send_signal(pid);   // Gửi thông báo cho Server

        printf("Data wirtten\n");
    }
    
    return 0;   
}
