#include "utils.h"

#ifdef HAVE_CLOCKGETTIME
#define PRECISION 1000000000;
#elif HAVE_GETTIMEOFDAY
#define PRECISION 1000000;
#else
#define PRECISION 1
#endif


//retrun timestamp in ns
unsigned long long get_time(){
#ifdef HAVE_CLOCKGETTIME
  struct timespec tp;
  clock_gettime (CLOCK_REALTIME, &tp);
  return (tp.tv_sec * 1000000000 + tp.tv_nsec);
#elif HAVE_GETTIMEOFDAY
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000000 + tv.tv_usec)*1000;
#endif
}


void my_sleep(unsigned long long length){
#ifdef HAVE_CLOCKGETTIME
  struct timespec tp;
  tp.tv_sec=length/1000000000;
  tp.tv_nsec=(length - tp.tv_sec*1000000000);
  //printf("sleep for %lu s and %lu ns \n", tp.tv_sec, tp.tv_nsec);
  nanosleep(&tp, NULL);
#elif HAVE_GETTIMEOFDAY
  usleep(length/1000);
#endif
}

void print_in_file(FILE *file, const char* func, int count, unsigned long long start, unsigned long long result){
  fprintf(file, "%s,%d,%f,%e\n", func, count, start/PRECISION, result/PRECISION);
}
