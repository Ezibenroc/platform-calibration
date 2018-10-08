#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

#ifdef HAVE_CLOCKGETTIME
#define PRECISION 1000000000.0
#elif HAVE_GETTIMEOFDAY
#define PRECISION 1000000.0
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

void print_in_file(FILE *file, const char* func, int *sizes, int nb_sizes, unsigned long long start, unsigned long long result){
  fprintf(file, "%s,", func);
  for(int i = 0; i < nb_sizes; i++) {
    fprintf(file, "%d,", sizes[i]);
  }
  fprintf(file, "%f,%e\n", start/PRECISION, result/PRECISION);
}

int op_from_string(const char *all_names[], const char *op_name) {
  for(int i = 0; all_names[i]; i++) {
    if(strcmp(op_name, all_names[i])==0)
      return i;
  }
  fprintf(stderr, "Unknown operation %s\n", op_name);
  exit(1);
}

int is_blank(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

// Trimming function, taken from https://stackoverflow.com/a/122721/4110059
char *trimwhitespace(char *str) {
  char *end;
  // Trim leading space
  while(is_blank(*str)) str++;
  if(*str == 0)  // All spaces?
    return str;
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && is_blank(*end)) end--;
  // Write new null terminator character
  end[1] = '\0';
  return str;
}

void error_expfile(const char *msg) {
  fprintf(stderr, "Wrong experiment file: %s\n", msg);
  exit(1);
}

experiment_t *parse_experiment_file(const char *all_names[], const char *filename, int *nb_exp, int *largest_size,
                                    int min_size, int max_size, int nb_sizes) {
  const int string_size = 100;
  char *tmp = malloc(string_size);
  char *old=tmp;
  int buffsize = 100;
  int offset = 0;
  assert(nb_sizes > 0 && nb_sizes <= MAX_NB_SIZES);
  char *tokens[nb_sizes+1];
  experiment_t *buff = (experiment_t*)malloc(buffsize*sizeof(experiment_t));
  assert(buff);
  FILE *file = fopen(filename, "r");
  if(file==NULL)
    error_expfile("could not open it");

  *largest_size = 0;
  while (fgets(tmp, string_size, file)) {
    for(int i = 0; i <= nb_sizes; i++) {
      tokens[i] = trimwhitespace(strsep(&tmp, ","));
    }
    buff[offset].op_id = op_from_string(all_names, tokens[0]);
    int accepted_entry = 1;
    int new_size_max=-1;
    for(int i = 1; i <= nb_sizes; i++) {
      int new_size = atoi(tokens[i]);
      buff[offset].sizes[i-1] = new_size;
      if(new_size < min_size || new_size > max_size) {
        accepted_entry = 0;
      }
      if(new_size_max < new_size) {
        new_size_max = new_size;
      }
    }
    if(accepted_entry) {
      offset ++;
      if(*largest_size < new_size_max)
        *largest_size = new_size_max;
      if(offset == buffsize) {
        buffsize *= 2;
        buff = (experiment_t*)realloc(buff, buffsize*sizeof(experiment_t));
        assert(buff);
      }
    }
    tmp = old;
  }
  free(tmp);
  fclose(file);
  *nb_exp = offset;
  return buff;
}
