#include "shared_memory.h"
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


#define SHM_SIZE 4096

const char *shm_name = "/my_shared_memory";


void shared_memory_write(double *cpu_usage, double *mem_usage) {
    int shm_fd;
    void *shm_ptr;
    // Tạo vùng nhớ cần chia sẻ
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);    

    if (shm_fd < 0) {
        perror("shm_open (write)");
        return;
    }

    // Xin kernel dự trữ frame
    if (ftruncate(shm_fd, SHM_SIZE) < 0) {
        perror("ftruncate");
        return;
    }

    // Xin cấp phát Virtual Page mapping vào Physical Page
    shm_ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0); // Chỉ Ghi và Shared
    if (shm_ptr == MAP_FAILED) {
        perror("mmap (write)");
        return;
    }

    // Ghi vào dùng shared memory
    double *data = (double *)shm_ptr;   
    *(data + 0) = *cpu_usage;
    *(data + 1) = *mem_usage;

    // Unmap và close the shared memory
    munmap(shm_ptr, SHM_SIZE);
    // close(shm_fd);  
}

void shared_memory_read(double *get_data_cpu, double *get_data_mem) {
    int shm_fd; 
    void *shm_ptr;

    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd < 0) {
        perror("shm_open (read)");
        return;
    }

    shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap (read)");
        return;
    }

    double *data = (double *)shm_ptr;
    *get_data_cpu = *(data + 0);
    *get_data_mem = *(data + 1);

    munmap(shm_ptr, SHM_SIZE);
    // close(shm_fd);

    //shm_unlink(shm_name); // Thoát khỏi mới dùng
}