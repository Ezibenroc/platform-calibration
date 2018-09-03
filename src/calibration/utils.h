#ifndef __CALIBRATE_UTILS_
#define __CALIBRATE_UTILS_

#include <stdio.h>

unsigned long long get_time();

void my_sleep(unsigned long long length);

void print_in_file(FILE *file, const char* func, int count, unsigned long long start, unsigned long long result);

#endif // __CALIBRATE_UTILS_
