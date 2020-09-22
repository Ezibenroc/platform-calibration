#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "utils.h"

#define MAX_NAME_SIZE 100
#define NB_RUNS 10

static void *src_buffer;
static void *dst_buffer;


FILE *open_file(const char *filename){
    FILE *file = fopen(filename, "w");
    if(!file) {
        perror("open_file");
        exit(errno);
    }
    return file;
}

void get_memcpy(FILE *file, int size, int nb_it, unsigned long long base_time) {
    for(int i=0; i<nb_it; i++) {
        int args[] = {size};
        int nb_args = sizeof(args)/sizeof(args[0]);
        unsigned long long start_time=get_time();
        memcpy(dst_buffer, src_buffer, size);
        unsigned long long total_time=get_time()-start_time;
        print_in_file(file, "memcpy", args, nb_args, start_time-base_time, total_time);
    }
}

static const char *names[] = {"memcpy", NULL};
static const void (*functions[])(FILE*, int, int, unsigned long long) = {get_memcpy};

void test_op(FILE *result_file, experiment_t *exp, int nb_runs, unsigned long long base_time) {
    functions[exp->op_id](result_file, exp->sizes[0], nb_runs, base_time);
}

void *allocate_buffer(int size) {
    char *buffer = (void*)malloc(size);
    if(buffer == NULL){
        fprintf(stderr, "Can't allocate memory (%g GB), decrease the max size\n",
            size*1e-9);
        perror("malloc");
        exit(1);
    }
    for(int i = 0; i < size; i++) {
        buffer[i] = rand();
    }
    return(buffer);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Syntax: %s <input_file> <output_file>\n", argv[0]);
        exit(1);
    }
    const char *input = argv[1];
    const char *output = argv[2];

    int nb_exp, largest_size;
    experiment_t *experiments = parse_experiment_file(names, input, &nb_exp, &largest_size, -1, (int)2e9, 1);
    FILE *result_file = open_file(output);
    src_buffer = allocate_buffer(largest_size);
    dst_buffer = allocate_buffer(largest_size);

    unsigned long long base_time=get_time();
    for(int j = 0; j < nb_exp; j++) {
        test_op(result_file, &experiments[j], NB_RUNS, base_time);
    }

    fflush(result_file);
    fclose(result_file);
    free(experiments);
    free(src_buffer);
    free(dst_buffer);
    return 0;
}
