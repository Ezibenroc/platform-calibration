#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include "utils.h"

#ifdef USE_MKL
#pragma message "Using the MKL for BLAS."
#include <mkl.h>
#else
#pragma message "Not using the MKL for BLAS."
#include <cblas.h>
#endif

#define MAX_LINES 2000
#define NB_RUNS 1
#define MAX_NAME_SIZE 256

#define MAX_OFFSET 1000000
#define WRITE_MEMORY_SIZE 100000000

FILE* active_file = NULL;
char* basename = "calibration";
char* dir_name  = ".";

static double *matrix_A;
static double *matrix_B;
static double *matrix_C;

unsigned long long base_time;

typedef struct {
  char* sizefile;
  int loop;
  char* resultfile;
} my_args;

static char doc[] = "Runs BLAS benchmarks to compute values used for SMPI calibration";


static struct argp_option options[] = {
  {"sizeFile", 's', "SIZEFILE", 0, "filename of the size list"},
  {"resultfile", 'o', "RESULTFILE", 0, "filename of the results"},
  {"loop", 'l', "NB_LOOPS", 0, "number of (non-recorded) loops to run before and after the execution"},
  { 0 }
};

static int parse_options (int key, char *arg, struct argp_state *state)
{
  my_args *arguments = state->input;
  switch (key){
  case 'l':
    arguments->loop = atoi(arg);
    break;
  case 's':
    arguments->sizefile = arg;
    break;
  case 'o':
    arguments->resultfile = arg;
    break;

  default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

struct argp argp = { options, parse_options, NULL, doc };
my_args arguments;

double *allocate_matrix(int size) {
    size_t alloc_size = (size*size + MAX_OFFSET) * sizeof(double);
    double *result = (double*) malloc(alloc_size);
    if(!result) {
      perror("malloc");
      exit(errno);
    }
    for(int i = 0; i < size*size; i++) {
        result[i] = (double)rand()/(double)(RAND_MAX);
    }
    return result;
}

void write_memory(void) {
    static char *buff = NULL;
    if(!buff) {
        buff = malloc(WRITE_MEMORY_SIZE);
        if(!buff) {
            perror("malloc");
            exit(errno);
        }
    }
    memset(buff, rand()%256, WRITE_MEMORY_SIZE);
}

FILE *open_file(const char* filename){
    FILE *file = fopen(filename, "w");
    if(!file) {
        perror("open_file");
        exit(errno);
    }
    return file;
}

int max3(const int *sizes) {
  int m = sizes[0];
  if(m < sizes[1])
    m = sizes[1];
  if(m < sizes[2])
    m = sizes[2];
  return m;
}

void get_dgemm(FILE *file, int *sizes, int nb_it, unsigned long long base_time, int write_file) {
  unsigned long long start_time, total_time;
  double alpha = 1., beta=1.;
  int max_size = max3(sizes);
  for(int i=0; i<nb_it; i++) {
    start_time=get_time();
    size_t offset = rand() % MAX_OFFSET;
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, sizes[0], sizes[1], sizes[2], alpha, matrix_A+offset, sizes[3],
            matrix_B+offset, sizes[4], beta, matrix_C+offset, sizes[5]);
    total_time=get_time()-start_time;
    if(write_file)
      print_in_file(file, "dgemm", sizes, 6, start_time-base_time, total_time);
  }
}

static const char *names[] = {"dgemm", NULL};
static const void (*functions[])(FILE*, int*, int, unsigned long long, int) = {get_dgemm};

void test_op(FILE *result_file, experiment_t *exp, int nb_runs, unsigned long long base_time, int write_file) {
  functions[exp->op_id](result_file, exp->sizes, nb_runs, base_time, write_file);
}

int main(int argc, char** argv){

  srand(42);
  bzero (&arguments, sizeof(my_args));

  if (argp_parse (&argp, argc, argv, 0, 0, &arguments) == ARGP_KEY_ERROR){
    fprintf(stderr,"error during the parsing of parameters\n");
    return 1;
  }

  int nb_loops = arguments.loop;

  if(arguments.sizefile==NULL){
    fprintf(stderr, "Please provide a name for a file containing a list of sizes\n");
    return -1;
  }
  if(arguments.resultfile==NULL){
    fprintf(stderr, "Please provide a name for a result file\n");
    return -1;
  }

  FILE *result_file = NULL;
  if(arguments.resultfile)
    result_file = open_file(arguments.resultfile);

  int nb_exp, largest_size;

  experiment_t *experiments = parse_experiment_file(names, arguments.sizefile, &nb_exp, &largest_size, -1, 100000, 6);

  printf("nb_exp=%d, largest_size=%d\n", nb_exp, largest_size);

  int max_size = largest_size*largest_size;
  printf("Alloc size: %.2e bytes\n", (double)(max_size)*sizeof(double)*3);

  matrix_A = allocate_matrix(largest_size);
  matrix_B = allocate_matrix(largest_size);
  matrix_C = allocate_matrix(largest_size);

  //Init time
  base_time=get_time();
  for(int i = 0; i < nb_loops; i++) {
    for(int j = 0; j < nb_exp; j++) {
      test_op(result_file, &experiments[j], NB_RUNS, base_time, 0);
    }
  }
  for(int j = 0; j < nb_exp; j++) {
    test_op(result_file, &experiments[j], NB_RUNS, base_time, 1);
  }
  for(int i = 0; i < nb_loops; i++) {
    for(int j = 0; j < nb_exp; j++) {
      test_op(result_file, &experiments[j], NB_RUNS, base_time, 0);
    }
  }

  fflush(result_file);
  fclose(result_file);
  free(experiments);
  free(matrix_A);
  free(matrix_B);
  free(matrix_C);
  return 0;
}
