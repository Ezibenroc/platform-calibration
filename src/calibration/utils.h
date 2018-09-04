#include <stdio.h>

#ifndef __CALIBRATE_UTILS_
#define __CALIBRATE_UTILS_

#include "unistd.h"
#if _POSIX_TIMERS
#include <time.h>
#define HAVE_CLOCKGETTIME 1
#else
#include <sys/time.h>
#define HAVE_GETTIMEOFDAY 1
#endif

unsigned long long get_time();

void my_sleep(unsigned long long length);

void print_in_file(FILE *file, const char* func, int count, unsigned long long start, unsigned long long result);

typedef struct {
  int op_id;
  int size;
} experiment_t;

experiment_t *parse_experiment_file(const char *all_names[], const char *filename, int *nb_exp, int *largest_size,
                                    int min_size, int max_size);

#endif // __CALIBRATE_UTILS_
