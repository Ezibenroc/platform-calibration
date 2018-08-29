#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if(argc != 2) {
        fprintf(stderr, "Syntax: %s <size>\n", argv[0]);
        exit(1);
    }
    int size = atoi(argv[1]);

    if (my_rank == 0) {
        char *send_buffer = (char *)calloc(size, sizeof(char));
        MPI_Request request;
        MPI_Isend(send_buffer, size, MPI_CHAR, (1-my_rank), 0,
        MPI_COMM_WORLD, &request);
        // Corrupt data
        for (int i=0; i < size; i++)
            send_buffer[i] = 1;

        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
    else {
        sleep(1);
        char *recv_buffer = (char *)calloc(size, sizeof(char));
        MPI_Recv(recv_buffer, size, MPI_CHAR, (1-my_rank), 0,
        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char corrupted = 0 ;
        for (int i=0; i < size; i++) {
            if (recv_buffer[i]) {
                corrupted = 1;
                break;
            }
        }

        if (corrupted) {
            fprintf(stdout,"corrupted\n");
        }
        else {
            fprintf(stdout,"ok\n");
        }
    }

    MPI_Finalize();
}
