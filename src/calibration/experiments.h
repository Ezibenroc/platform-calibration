#ifndef __EXPERIMENTS_H_
#define __EXPERIMENTS_H_

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h>
#include "calibrate.h"

double get_Recv(int count, int nb_it);
double get_iSendtime(int count, int nb_it);
double get_PingPongtime(int count, int nb_it);
long get_Wtime(int count, int nb_it);
double get_Iprobe(int count, int nb_it);
double get_Test(int count, int nb_it);

#endif //__EXPERIMENTS_H_
