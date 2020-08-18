#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <mpi.h>
#include "utils.h"

#define MAX_NAME_SIZE 100
#define NB_RUNS 10

static void *my_send_buffer;
static void *my_recv_buffer;


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


void get_ring(FILE *file, int size, int nb_it, unsigned long long base_time) {
    static int my_rank = -1;
    static int nb_ranks = -1;
    if(my_rank < 0) {
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &nb_ranks);
    }
    int args[] = {my_rank, size};
    unsigned long long start_time, total_time;
    double alpha = 1., beta=1.;
    for(int i=0; i<nb_it; i++) {
        int recv_from = (my_rank-1+nb_ranks)%nb_ranks;
        int send_to = (my_rank+1)%nb_ranks;
// I am the root of the broadcast, I send
        if(my_rank == 0) {
            start_time=get_time();
            MPI_Send(my_send_buffer, size, MPI_CHAR, send_to, 0, MPI_COMM_WORLD);
            total_time=get_time()-start_time;
            print_in_file(file, "MPI_Send", args, sizeof(args)/sizeof(args[0]), start_time-base_time, total_time);
        }
// I receive
        int flag = 0;
        while(!flag) {
            MPI_Iprobe(recv_from, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        }
        start_time=get_time();
        MPI_Recv(my_recv_buffer, size, MPI_CHAR, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        total_time=get_time()-start_time;
        print_in_file(file, "MPI_Recv", args, sizeof(args)/sizeof(args[0]), start_time-base_time, total_time);
// I am *not* the root of the broadcast, I send
        if(my_rank != 0) {
            start_time=get_time();
            MPI_Send(my_send_buffer, size, MPI_CHAR, send_to, 0, MPI_COMM_WORLD);
            total_time=get_time()-start_time;
            print_in_file(file, "MPI_Send", args, sizeof(args)/sizeof(args[0]), start_time-base_time, total_time);
        }
    }
}

static const char *names[] = {"ring", NULL};
static const void (*functions[])(FILE*, int, int, unsigned long long) = {get_ring};

void test_op(FILE *result_file, experiment_t *exp, int nb_runs, unsigned long long base_time) {
    functions[exp->op_id](result_file, exp->sizes[0], nb_runs, base_time);
}

void *allocate_buffer(int size) {
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
    return 0;
}
