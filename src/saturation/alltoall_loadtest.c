
/* 
 *
 *
 *
 * Four loadtests are performed in this file. Each one of them begins with a few 
 * nodes sending data, and finish with all of them sending at the same time.
 * for each test except the third one, the pattern is the same
 *
 * Test 1 : a simple send/recv, incrementing from one node sending to half of 
 * them sending to the other half, each time adding one. this tests the full 
 * capacity of the link (n steps at the end)
 *
 * Test 2 : a sendrecv, incrementing from one sender each time, tests the fullduplex 
 * capacity of the links (n steps at the end)
 *
 * Test 3 : have each process send and receive at the same time, but from a 
 * different node. Here is a wonderful ascii representation to show the pattern
 * A-> B
   ^  /
 *  \/
 *  /\
 * v  \
 * C-> D
 * So at each step we add two more nodes at each side (n/2 steps at the end)
 *
 * Test 4 : do an alltoall between all nodes, adding two each time, (n steps at the end)
 *
 *
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "unistd.h"
#if _POSIX_TIMERS
#include <time.h>
#define HAVE_CLOCKGETTIME 1
#else
#include <sys/time.h>
#define HAVE_GETTIMEOFDAY 1
#endif

#include "mpi.h"





#define NUMTESTS 4

#define MAX_NAME_SIZE 256
#define MAX_LINE_SIZE 512

#define CHUNK_SIZE 100*MAX_LINE_SIZE

#define NB_RUNS 10

char* active_buffer = NULL;
unsigned long long base_time;
int testid = 0;
int offset = 0;
int nb_chunks = 1;

void print_in_buffer(int rank, const char* func, int count, unsigned long long start_time, unsigned
long long total_time);
void switch_buffer();

 
void switch_buffer(){

    int topo;
    int rank, size;
    int i;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    int* sizebuff=NULL;
    int* disps=NULL;
    if(rank==0){
      sizebuff=malloc(sizeof(int)*size);
      disps=malloc(sizeof(int)*size);
    }
      
    //get the sizes of each buffer
    MPI_Gather(&offset, 1, MPI_INT, sizebuff, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int totsize=0;
    if(rank==0){
      for(i=0; i<size; i++){
        disps[i]=totsize;
        totsize+=sizebuff[i];
      }
    }

    char* buff=NULL;
    if(rank==0)
      buff=malloc(totsize*sizeof(char));
      
    MPI_Gatherv(active_buffer, offset, MPI_CHAR, buff, sizebuff, disps, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    //rank 0 dumps the file
    if(rank==0){
      char* filename= malloc(MAX_NAME_SIZE*sizeof(char));
      char* name;
      switch(testid){
      case 0:
        name="send";
      break;
      case 1:
        name="sendrecv_same";
      break;
      case 2:
        name="sendrecv_diff";
      break;
      case 3:
        name="alltoall";
      break;
      }
      snprintf(filename,MAX_NAME_SIZE, "load_%s.csv", name);
      FILE* active_file=fopen(filename, "w");
      fwrite (buff , sizeof(char), totsize, active_file);

      fflush(active_file);
      fclose(active_file);
    }
    
    //memset(active_buffer, '\0', offset);
    offset=0;
    MPI_Barrier(MPI_COMM_WORLD);
}

//for the switch buffer we want real buffers, 
#ifndef SMPI_SHARED_MALLOC
#define SMPI_SHARED_MALLOC malloc
#define SMPI_SHARED_FREE free 
#endif


double precision = 1000000;

//retrun timestamp in ns
unsigned long long get_time(){
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000000 + tv.tv_usec)*1000;
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
extern smpi_loaded_page;
void print_in_buffer(int rank, const char* func, int count, unsigned long long start, unsigned long long resul){
  if(offset+MAX_LINE_SIZE>nb_chunks*CHUNK_SIZE){
    nb_chunks++;
    active_buffer=realloc(active_buffer, nb_chunks*CHUNK_SIZE*sizeof(char));
  }
  offset+=snprintf(active_buffer+offset,MAX_LINE_SIZE, "%d,%s,%d,%f,%e\n",rank, func, count, (start-base_time)/precision, resul/precision);
}

int main( int argc, char **argv )
{
  MPI_Status status;
  int count, rank, size,  dest, source, i, err = 0, toterr;
  int *inbuf;
  int *outbuf;

  /* Initialize MPI and get my rank and total number of
     processors */  
  MPI_Init(&argc, &argv);
  
  //allocate the buffer with one chunk
  active_buffer=malloc(CHUNK_SIZE*sizeof(char));
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  count  = 4000000; //4MB messages should be enough
  inbuf    = (int *)SMPI_SHARED_MALLOC(size* count * sizeof(int) );
  outbuf    = (int *)SMPI_SHARED_MALLOC(size* count * sizeof(int) );
  int x=0;
  int half=size/2;  
  base_time=get_time();
  //warmup
  MPI_Alltoall(NULL, 0, MPI_CHAR, NULL, 0, MPI_CHAR, MPI_COMM_WORLD);
  
  for (i=0; i<count; i++)
    inbuf[i] = rank + size*i;
  unsigned long long start_time, call_time;
  for (testid=0; testid<NUMTESTS; testid++){
    for(x=0; x<half;x++){
      if(rank==0)
        printf("test %d, testing %d processes\n", testid, (x+1)*2);

      switch (testid){
        case 0:
          for (i=0;i<NB_RUNS;i++){
            if(rank<=x){
              start_time = get_time();
              MPI_Send( inbuf, count, MPI_CHAR, rank+half,  1, MPI_COMM_WORLD );
              call_time = get_time() - start_time;
              print_in_buffer(rank,"MPI_Send",count, start_time, call_time );
            }else if (rank >= half && rank-half<=x){
              start_time = get_time();
              MPI_Recv( outbuf, count, MPI_CHAR, rank-half, 1, MPI_COMM_WORLD, &status );
              call_time = get_time() - start_time;
              print_in_buffer(rank,"MPI_Send",count, start_time, call_time );
            }
          }
        break;
        case 1:
          for (i=0;i<NB_RUNS;i++){
            if(rank<=x){
              start_time = get_time();
              MPI_Sendrecv( inbuf, count, MPI_CHAR, rank+half,  1, outbuf, count, MPI_CHAR, rank+half, 1, MPI_COMM_WORLD, &status );
              call_time = get_time() - start_time;
              print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
            }else if(rank >= half && rank-half<=x){
              start_time = get_time();
              MPI_Sendrecv( outbuf, count, MPI_CHAR, rank-half, 1, outbuf, count, MPI_CHAR, rank-half, 1,  MPI_COMM_WORLD, &status );
              call_time = get_time() - start_time;
              print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
            }
          }
        break;
        case 2:
          if(!(x%2))break;//need to increment by two for this pattern
          for (i=0;i<NB_RUNS;i++){
            if(rank<=x){
              if(rank%2 ==0){
                start_time = get_time();
//        printf("rank %d sending to %d receiving from %d\n", rank, rank+half,  (rank+1)+half); 
                MPI_Sendrecv( inbuf, count, MPI_CHAR, rank+half,  1, outbuf, 
                  count, MPI_CHAR, (rank+1)+half, 1, MPI_COMM_WORLD, &status );
                call_time = get_time() - start_time;
                print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
              }else{
                start_time = get_time();
  //              printf("rank %d sending to %d receiving from %d\n", rank, rank+half,  (rank-1)+half); 
                MPI_Sendrecv( inbuf, count, MPI_CHAR, rank+half,  1, outbuf, 
                  count, MPI_CHAR, (rank-1)+half, 1, MPI_COMM_WORLD, &status );
                call_time = get_time() - start_time;
                print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
              }
            }else if(rank >= half && rank-half<=x){
              if((rank-half)%2==0){
                start_time = get_time();
    //            printf("rank %d sending to %d receiving from %d\n", rank, rank+1-half,  rank-half); 
                MPI_Sendrecv( outbuf, count, MPI_CHAR, rank+1-half, 1, outbuf, 
                  count, MPI_CHAR, rank-half, 1,  MPI_COMM_WORLD, &status );
                call_time = get_time() - start_time;
                print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
              }else{
                start_time = get_time();
      //          printf("rank %d sending to %d receiving from %d\n", rank, rank-1-half,  rank-half); 
                MPI_Sendrecv( outbuf, count, MPI_CHAR, rank-1-half, 1, outbuf, 
                  count, MPI_CHAR, rank-half, 1,  MPI_COMM_WORLD, &status );
                call_time = get_time() - start_time;
                print_in_buffer(rank,"MPI_Sendrecv",count, start_time, call_time );
              }
            }
          }
        break;
        case 3:
          for (i=0;i<3;i++){
            MPI_Group g1,g2;
            MPI_Comm comm1;
            MPI_Comm_group( MPI_COMM_WORLD, &g1 );
            int ranks[2*(x+1)];
            int j=0;
            for (j =0; j<2*(x+1); j++)
              ranks[j]=j;
            
            MPI_Group_incl( g1, 2*(x+1), ranks, &g2 );
            int size1, rank1;
            MPI_Group_size( g2, &size1 );
            MPI_Group_rank( g2, &rank1 );
            
            if (size1 != 2*(x+1)) {
              fprintf( stderr, "Size should be %d, is %d\n", 2*(x+1), size1 );fflush(stderr);
            }
            
            if (rank<2*(x+1) && rank1 != rank) {
              fprintf( stderr, "Rank should be %d, is %d\n", rank, rank1 );fflush(stderr);
              exit(0);
            }
            
            MPI_Comm_create(MPI_COMM_WORLD,  g2,  &comm1);
            if(rank<2*(x+1)){
              start_time = get_time();
              MPI_Alltoall(outbuf, count, MPI_CHAR, inbuf, count, MPI_CHAR, comm1);
              call_time = get_time() - start_time;
              print_in_buffer(rank,"MPI_Alltoall",count, start_time, call_time );
            }
            if(comm1!=MPI_COMM_NULL)MPI_Comm_free(&comm1);
          }
        break;
        defaut:
        break;
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    switch_buffer();
  }
/*
    fprintf(stderr, "Done with SRR on proc %d\n", rank);
 */
 
 

    /* Finalize everything */
  SMPI_SHARED_FREE( outbuf );
  SMPI_SHARED_FREE( inbuf );
  MPI_Finalize();
  return 0;
}
