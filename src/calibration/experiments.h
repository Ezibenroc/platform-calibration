#ifndef __EXPERIMENTS_H_
#define __EXPERIMENTS_H_

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h>
#include "calibrate.h"

void get_Recv(FILE *file, int count, int nb_it, unsigned long long base_time);
void get_Isend(FILE *file, int count, int nb_it, unsigned long long base_time);
void get_PingPong(FILE *file, int count, int nb_it, unsigned long long base_time);
void get_Wtime(FILE *file, int count, int nb_it, unsigned long long base_time);
void get_Iprobe(FILE *file, int count, int nb_it, unsigned long long base_time);
void get_Test(FILE *file, int count, int nb_it, unsigned long long base_time);

#endif //__EXPERIMENTS_H_
