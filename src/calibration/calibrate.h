#ifndef __CALIBRATE_H_
#define __CALIBRATE_H_

#include "mpi.h"
#include "utils.h"
#include "experiments.h"
void* get_send_buffer();
void* get_recv_buffer();
unsigned long long get_time();
void my_sleep(unsigned long long length);

MPI_Comm get_comm();
int get_rank();

#endif // __CALIBRATE_H_
