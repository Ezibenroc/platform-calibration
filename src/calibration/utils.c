#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
                                    int min_size, int max_size) {
  const int string_size = 100;
  char *tmp = malloc(string_size);
  char *token1, *token2, *old=tmp;
  int buffsize = 100;
  int offset = 0, new_size, new_op;
  int size_first;
  experiment_t *buff = (experiment_t*)malloc(buffsize*sizeof(experiment_t));
  assert(buff);
  FILE *file = fopen(filename, "r");
  if(file==NULL)
    error_expfile("could not open it");

  *largest_size = 0;
  if(!fgets(tmp, string_size, file))
    error_expfile("file is empty");
  token1 = trimwhitespace(strsep(&tmp, ","));
  if(!tmp)
    error_expfile("header has only one column");
  token2 = trimwhitespace(strsep(&tmp, ","));
  if(tmp)
    error_expfile("header has more than two columns");
  if(!strcmp(token1, "operation") && !strcmp(token2, "size")) {
    size_first = 0;
  }
  else if(!strcmp(token1, "size") && !strcmp(token2, "operation")) {
    size_first = 1;
  }
  else {
    error_expfile("wrong header, expected column names 'size' and 'operation'\n");
  }
  tmp = old;
  while (fgets(tmp, string_size, file)) {
    token1 = trimwhitespace(strsep(&tmp, ","));
    token2 = trimwhitespace(strsep(&tmp, ","));
    if(size_first) {
      new_op = op_from_string(all_names, token2);
      new_size = atoi(token1);
    }
    else {
      new_op = op_from_string(all_names, token1);
      new_size = atoi(token2);
    }
    tmp = old;
    if(new_size >= min_size && new_size <= max_size) {
      if(*largest_size < new_size)
        *largest_size = new_size;
      buff[offset].op_id = new_op;
      buff[offset].size = new_size;
      offset ++;
      if(offset == buffsize) {
        buffsize *= 2;
        buff = (experiment_t*)realloc(buff, buffsize*sizeof(experiment_t));
        assert(buff);
      }
    }
  }
  free(tmp);
  fclose(file);
  *nb_exp = offset;
  return buff;
}
