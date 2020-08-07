#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
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

/*
 * Return 0b000...01...1, with n ones at the end.
 */
uint64_t __get_mask(unsigned n) {
    if(n == 0)
        return 0;
    assert(n >= 0 && n < 64);
    uint64_t mask = 1;
    return (mask << n) - 1;
}

/*
 * Return 0b000...01...10...0.
 * The bits at positions [start, stop] are equal to 1, the others to 0 (start is the lower order).
 */
uint64_t get_mask(unsigned start, unsigned stop) {
    assert(0 <= start && start <= stop && stop < 64);
    uint64_t ones = __get_mask(stop);
    uint64_t zeroes = ~__get_mask(start);
    return ones & zeroes;
}

/*
 * Apply the given mask to a double.
 */
double apply_mask(double x, uint64_t mask) {
    uint64_t *tmp = (uint64_t*)&x;
    (*tmp) |= mask;
    return *((double*)tmp);
}

void __print_bits(uint64_t n, unsigned i) {
    if(i == 64)
        return;
    __print_bits(n / 2, i+1);
    if(n % 2)
        printf("1");
    else
        printf("0");
}

void print_bits(uint64_t n) {
    printf("0b");
    __print_bits(n, 0);
    printf("\n");
}

void print_bits_f(double x) {
    uint64_t *tmp = (uint64_t*)&x;
    print_bits(*tmp);
}

double *allocate_matrix(size_t size1, size_t size2) {
    size_t alloc_size = (size1*size2 + MAX_OFFSET) * sizeof(double);
    double *result = (double*) malloc(alloc_size);
    if(!result) {
      perror("malloc");
      exit(errno);
    }
#ifdef MASK_SIZE
    uint64_t mask = get_mask(0, MASK_SIZE);
#pragma message "Using a mask for the values of the matrix."
#else
    // Note that this particular mask does not do anything: apply_mask(x, get_mask(0, 0)) == x
    // One need to change the stop and/or start indices to really apply a mask.
    uint64_t mask = get_mask(0, 0);
#endif
    printf("Using the mask: ");
    print_bits(mask);
    double x = 3.1415926535897932384626433;
    assert(x == apply_mask(x, get_mask(0, 0)));
    assert(x != apply_mask(x, get_mask(0, 1)));
    for(int i = 0; i < size1*size2; i++) {
        result[i] = apply_mask((double)rand()/(double)(RAND_MAX), mask);
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

void get_dgemm(FILE *file, int *sizes, int nb_it, unsigned long long base_time, int write_file) {
  unsigned long long start_time, total_time;
  double alpha = 1., beta=1.;
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


int get_max_size(experiment_t *exp, int nb_exp, int idx) {
  int max = -1;
  for(int i = 0; i < nb_exp; i++) {
    int size = exp[i].sizes[idx];
    if(max < size)
      max = size;
  }
  return max;
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

  experiment_t *experiments = parse_experiment_file(names, arguments.sizefile, &nb_exp, &largest_size, -1, 300000, 6);

  printf("nb_exp=%d, largest_size=%d\n", nb_exp, largest_size);

  size_t max_size = (size_t)largest_size*(size_t)largest_size;
  int max_M = get_max_size(experiments, nb_exp, 0);
  int max_N = get_max_size(experiments, nb_exp, 1);
  int max_K = get_max_size(experiments, nb_exp, 2);
  printf("max(M)=%d | max(N)=%d | max(K)=%d\n", max_M, max_N, max_K);
  printf("Alloc size: was %.2e bytes\n", (double)(max_size)*sizeof(double)*3);
  double alloc_size = (double)max_M*(double)max_K;
  alloc_size += (double)max_K*(double)max_N;
  alloc_size += (double)max_M*(double)max_N;
  printf("Alloc size: now %.2e bytes\n", alloc_size*sizeof(double));

  matrix_A = allocate_matrix(max_M, max_K);
  matrix_B = allocate_matrix(max_K, max_N);
  matrix_C = allocate_matrix(max_M, max_N);

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
