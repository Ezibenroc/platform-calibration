#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <mpi.h>
#include <cblas.h>
#include "utils.h"

#ifndef MATRIX_SIZE
#define MATRIX_SIZE 128
#pragma message "Using default matrix size"
#endif
#define MIN_BUFF_SIZE MATRIX_SIZE*MATRIX_SIZE*sizeof(double)
#ifndef REUSE_BUFFER
#define REUSE_BUFFER 1
#endif
#if REUSE_BUFFER
#pragma message "Using the same buffers for communications and computations"
#else
#pragma message "Using different buffers for communications and computations"
#endif
#define MAX_NAME_SIZE 100
#define NB_RUNS 10

static void *my_send_buffer;
static void *my_recv_buffer;
static void *aux_buffer;
static double *matrix_A, *matrix_B, *matrix_C;
static int op_id = 0;


FILE *open_file(const char *dir_name, const char *file_prefix){
    char* filename= malloc(MAX_NAME_SIZE*sizeof(char));
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    sprintf(filename, "%s/%s_%d.csv", dir_name, file_prefix, my_rank);
    FILE *file = fopen(filename, "w");
    if(!file) {
        perror("open_file");
        fprintf(stderr, "Maybe directory %s does not exist?\n", dir_name);
        exit(errno);
    }
    free(filename);
    MPI_Barrier(MPI_COMM_WORLD);
    return file;
}


void send_msg(int size, int dst, FILE *file, int args[], int nb_args, unsigned long long base_time) {
    unsigned long long start_time=get_time();
    MPI_Send(my_send_buffer, size, MPI_CHAR, dst, 0, MPI_COMM_WORLD);
    unsigned long long total_time=get_time()-start_time;
    print_in_file(file, "MPI_Send", args, nb_args, start_time-base_time, total_time);
}

void recv_msg(int size, int src, FILE *file, int args[], int nb_args, unsigned long long base_time, int busy_waiting) {
    int flag = 0;
    // Meanwhile we are waiting for a message to be there, we call dgemm
    while(busy_waiting & !flag) {
        cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE, 1., matrix_A,
                MATRIX_SIZE, matrix_B, MATRIX_SIZE, 1., matrix_C, MATRIX_SIZE);
        MPI_Iprobe(src, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    }
    unsigned long long start_time=get_time();
    MPI_Recv(my_recv_buffer, size, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    unsigned long long total_time=get_time()-start_time;
    print_in_file(file, "MPI_Recv", args, nb_args, start_time-base_time, total_time);
}

void get_ring(FILE *file, int size, int nb_it, unsigned long long base_time) {
    static int my_rank = -1;
    static int nb_ranks = -1;
    if(my_rank < 0) {
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &nb_ranks);
    }
    for(int i=0; i<nb_it; i++) {
        int args[] = {my_rank, size, op_id++};
        int recv_from = (my_rank-1+nb_ranks)%nb_ranks;
        int send_to = (my_rank+1)%nb_ranks;
// I am the root of the broadcast, I send
        if(my_rank == 0) {
            send_msg(size, send_to, file, args, sizeof(args)/sizeof(args[0]), base_time);
        }
// I receive
        recv_msg(size, recv_from, file, args, sizeof(args)/sizeof(args[0]), base_time, 1);
// I am *not* the root of the broadcast, I send
        if(my_rank != 0) {
            send_msg(size, send_to, file, args, sizeof(args)/sizeof(args[0]), base_time);
        }
    }
}

void get_ringrong(FILE *file, int size, int nb_it, unsigned long long base_time) {
    static int my_rank = -1;
    static int nb_ranks = -1;
    if(my_rank < 0) {
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &nb_ranks);
    }
    for(int i=0; i<nb_it; i++) {
        int args[] = {my_rank, size, op_id++};
        int prev = (my_rank-1+nb_ranks)%nb_ranks;
        int next = (my_rank+1)%nb_ranks;
        if(my_rank == 0) {
            send_msg(size, next, file, args, sizeof(args)/sizeof(args[0]), base_time);
            recv_msg(size, next, file, args, sizeof(args)/sizeof(args[0]), base_time, 0);
            recv_msg(size, prev, file, args, sizeof(args)/sizeof(args[0]), base_time, 1);
            send_msg(size, prev, file, args, sizeof(args)/sizeof(args[0]), base_time);
        }
        else {
            recv_msg(size, prev, file, args, sizeof(args)/sizeof(args[0]), base_time, 1);
            send_msg(size, prev, file, args, sizeof(args)/sizeof(args[0]), base_time);
            send_msg(size, next, file, args, sizeof(args)/sizeof(args[0]), base_time);
            recv_msg(size, next, file, args, sizeof(args)/sizeof(args[0]), base_time, 0);
        }
    }
}

static const char *names[] = {"Ring", "RingRong", NULL};
static const void (*functions[])(FILE*, int, int, unsigned long long) = {get_ring, get_ringrong};

void test_op(FILE *result_file, experiment_t *exp, int nb_runs, unsigned long long base_time) {
    functions[exp->op_id](result_file, exp->sizes[0], nb_runs, base_time);
}

void *allocate_buffer(int size) {
    if(size < MIN_BUFF_SIZE)
        size = MIN_BUFF_SIZE;
    char *buffer = (void*)malloc(size);
    if(buffer == NULL){
        fprintf(stderr, "Can't allocate memory (%g GB), decrease max size of the messages \n",
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
    printf("Matrix size: %d\n", MATRIX_SIZE);
    if(argc != 3) {
        fprintf(stderr, "Syntax: %s <input_file> <output_directory>\n");
        exit(1);
    }
    const char *input = argv[1];
    const char *output = argv[2];
    MPI_Init(&argc, &argv);


    int nb_exp, largest_size;
    experiment_t *experiments = parse_experiment_file(names, input, &nb_exp, &largest_size, -1, (int)1e9, 1);
    FILE *result_file = open_file(output, "result");
    my_recv_buffer = allocate_buffer(largest_size);
    my_send_buffer = allocate_buffer(largest_size);
    aux_buffer = allocate_buffer(largest_size);
#if REUSE_BUFFER
    matrix_A = my_recv_buffer;
    matrix_B = my_send_buffer;
    matrix_C = aux_buffer;
#else
    matrix_A = allocate_buffer(MIN_BUFF_SIZE);
    matrix_B = allocate_buffer(MIN_BUFF_SIZE);
    matrix_C = allocate_buffer(MIN_BUFF_SIZE);
#endif

    MPI_Barrier(MPI_COMM_WORLD);
    unsigned long long base_time=get_time();
    for(int j = 0; j < nb_exp; j++) {
        test_op(result_file, &experiments[j], NB_RUNS, base_time);
    }
    MPI_Finalize();

    fflush(result_file);
    fclose(result_file);
    free(experiments);
    free(my_recv_buffer);
    free(my_send_buffer);
    free(aux_buffer);
#if REUSE_BUFFER
#else
    free(matrix_A);
    free(matrix_B);
    free(matrix_C);
#endif
    return 0;
}
