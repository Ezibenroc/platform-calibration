#ifndef __CALIBRATE_H_
#define __CALIBRATE_H_

#include "mpi.h"
#include "experiments.h"
void* get_send_buffer();
void* get_recv_buffer();
unsigned long long get_time();
void my_sleep(unsigned long long length);
void print_in_file(FILE *file, const char* func, int count, unsigned long long start_time, unsigned long long total_time);

MPI_Comm get_comm();
int get_rank();

#endif // __CALIBRATE_H_
