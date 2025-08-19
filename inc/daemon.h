#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "message_queue.h"
#include "shared_memory.h"

// Thông tin cần thiết khi đọc từ /proc/stat 
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
} cpu_stat_t;

// Thông tin cần thiết khi đọc từ /proc/meminfo
typedef struct {
    unsigned long long memtotal, memavailable;
} mem_info_t;

/**
 * Lấy data từ /pro/stat và tính toán CPU usage (%)
 */
double get_cpu_usage(cpu_stat_t *stat1, cpu_stat_t *stat2);

/**
 * Lấy data từ /pro/meminfo và tính toán mem usage (%)
 */
double get_mem_usage(mem_info_t *info);

/**
 * Nhận data từ Server qua Message Queue
 */
void daemon_receive_data_mq(char *buffer);

/**
 * Lấy dữ liệu ghi vào vùng Shared Memory
 */
void daemon_write_data_shm(void);

/**
 * Gửi signal đến cho Server để báo là đã ghi xong
 */
void send_signal(pid_t pid);

#endif