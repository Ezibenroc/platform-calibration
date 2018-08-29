#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    if(argc != 2) {
        fprintf(stderr, "Syntax: %s <size>\n", argv[0]);
        exit(1);
    }
    int size = atoi(argv[1]);
    char *buffer = (char *)malloc(sizeof(char) * size);
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    MPI_Send(buffer, size, MPI_CHAR, (1-my_rank), 0, MPI_COMM_WORLD);
    MPI_Recv(buffer, size, MPI_CHAR, (1-my_rank), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Finalize();
    if (my_rank == 0) {
        fprintf(stdout,"ok\n");
    }
}
