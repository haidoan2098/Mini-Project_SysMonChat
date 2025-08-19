#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

#include <stdio.h>

void shared_memory_write(double *cpu_usage, double *mem_usage);
void shared_memory_read(double *get_data_cpu, double *get_data_mem);

#endif