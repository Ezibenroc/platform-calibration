/* In this file we will launch a bunch of MPI experiments, to profile the times
a real implementation takes to execute some calls. Timings should be done with
akypuera Library and then output to a script, which will generate the values to
apply to the platform file, in order to obtain a correct simulation. The time
synchronization should be done with rastro, in order to get good timestamps.



Profiled functions :
Recvtime :  time spent inside the irecv function, We have to insure that the
communication is over, so we use a sleep of the time of a full pinpong to insure completion)

iSendtime : time spent inside the isend function (buffering data and sending it to the network card)


PingPong Send/Receive : This is the one that will allow to compute the network
timings. As we already have the values for the receive and isends for the values
, we can compute the latency and bandwidth.
Pingpong also uses MPI_Send, so we can check here that its values are coherent with Isend

MPI_Test, MPI_Iprobe, MPI_Wtime  : we have to profile time spent in this functions as well


iRecvtime : time spent inside the irecv function. - NOT USED

We use a 10 runs of each function, each with n different sizes.
The sizes are taken from the file zoo_sizes
This set was generated in order to provide a comprehensive set of sizes,
with a good repartition for all magnitudes.
We can set the maximum limit for the sizes with -s (as sizes go up to 1GB in zoo)

*/

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include "calibrate.h"


#define MAX_LINES 2000
#define NB_RUNS 10
#define MAX_NAME_SIZE 256

static void * my_send_buffer;
static void * my_receive_buffer;

FILE* active_file = NULL;
char* basename = "calibration";
char* dir_name  = ".";

unsigned long long base_time;

typedef struct {
  char* sizefile;
  char* filename;
  int max_size;
  int min_size;
  int nb_runs;
  char* prefix;
  char* dir_name;
} my_args;



static char doc[] = "Runs MPI point to point benchmarks to compute values used for SMPI calibration";


static struct argp_option options[] = {
  {"sizeFile", 's', "SIZEFILE", 0, "filename of the size list"},
  {"min_size", 'm', "MIN SIZE", 0, "Minimum size to process"},
  {"max_size", 'M', "MAX SIZE", 0, "Maximum size to process"},
  {"nb_runs", 'n', "NB RUNS", 0, "number of times you want to execute the run"},
  {"prefix", 'p', "PREFIX", 0, "prefix of the csv files"},
  {"dir_name", 'd', "dir_name", 0, "Name/path of the directory to save files into. No trailing slashes."},
  { 0 }
};

/* Returns an integer in the range [0, n).
 * Code taken from https://stackoverflow.com/a/822361/4110059
 * Uses rand(), and so is affected-by/affects the same seed.
 */
int randint(int n) {
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Supporting larger values for n would requires an even more
    // elaborate implementation that combines multiple calls to rand()
    assert (n <= RAND_MAX);

    // Chop off all of the values that would cause skew...
    int end = RAND_MAX / n; // truncate skew
    assert (end > 0);
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n;
  }
}

static int parse_options (int key, char *arg, struct argp_state *state)
{
  my_args *arguments = state->input;
  switch (key){
  case 'm':
    arguments->min_size = atoi(arg);
    break;
  case 'M':
    arguments->max_size = atoi(arg);
    break;
  case 'n':
    arguments->nb_runs = atoi(arg);
    break;
  case 'p':
    arguments->prefix = arg;
    break;
   case 's':
    arguments->sizefile = arg;
    break;

  case 'f':
    arguments->filename = arg;
    break;
  case 'd':
    arguments->dir_name = arg;
    break;

  default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

struct argp argp = { options, parse_options, NULL, doc };
my_args arguments;
#ifdef HAVE_CLOCKGETTIME
double precision = 1000000000;
#elif HAVE_GETTIMEOFDAY
double precision = 1000000;
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

void* get_send_buffer(){
  return my_send_buffer;
}
void* get_recv_buffer(){
  return my_receive_buffer;
}

MPI_Comm get_comm(){
  return MPI_COMM_WORLD;
}

int get_rank(){
  int measurement_rank;
  MPI_Comm_rank(get_comm(), &measurement_rank);
  return measurement_rank;
}

int get_size(){
  int size;
  MPI_Comm_size(get_comm(), &size);
  return size;
}

FILE *open_file(char* name){
    char* filename= malloc(MAX_NAME_SIZE*sizeof(char));
    sprintf(filename, "%s/%s_%s.csv", dir_name, basename, name);
    FILE *file = fopen(filename, "w");
    free(filename);
    MPI_Barrier(MPI_COMM_WORLD);
    return file;
}

void print_in_file(FILE *file, const char* func, int count, unsigned long long start, unsigned long long resul){
  fprintf(file, "%s,%d,%f,%e\n", func, count, (start-base_time)/precision, resul/precision);
}


static const char *names[] = {"Recv", "Isend", "PingPong", "Wtime", "Iprobe", "Test", NULL};
static const void (*functions[])(FILE*, int, int) = {get_Recv, get_Isend, get_PingPong, get_Wtime,
                                                 get_Iprobe, get_Test};

int op_from_string(const char *op_name) {
  for(int i = 0; names[i]; i++) {
    if(strcmp(op_name, names[i])==0)
      return i;
  }
  fprintf(stderr, "Unknown MPI operation %s\n", op_name);
  exit(1);
}

void test_op(FILE *output_files[], const char *op_name, int size, int nb_runs) {
  int op_id = op_from_string(op_name);
  functions[op_id](output_files[op_id], size, nb_runs);
}

int main(int argc, char** argv){

  bzero (&arguments, sizeof(my_args));


  if (argp_parse (&argp, argc, argv, 0, 0, &arguments) == ARGP_KEY_ERROR){
    fprintf(stderr,"error during the parsing of parameters\n");
    return 1;
  }

  if(arguments.nb_runs ==0)arguments.nb_runs=1;

  if(arguments.prefix!=NULL)
    basename=arguments.prefix;
  if(arguments.dir_name!=NULL) {
    dir_name=arguments.dir_name;
  }
  if(arguments.sizefile==NULL){
    printf("Please provide a name for a file containing a list of sizes\n");
    return -1;
  }
  //
// load sizes from file
  int sizes[MAX_LINES*arguments.nb_runs];
  int index;
  int my_rank;
  int size;
  int m=0;

  MPI_Init(&argc, &argv);

  FILE *output_files[6];
  for(int i = 0; names[i]; i++) {
    output_files[i] = open_file(names[i]);
  }


  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if(size<2){
    printf("Please run with at least 2 processes, you have only %d\n", size);
    return -1;
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  printf("[%d] MPI initialized\n", get_rank());

  FILE* fin;
  fin=fopen(arguments.sizefile, "r");
  if(fin==NULL) {
      printf("Error opening file %s \n", arguments.sizefile);
      return -1;
  }
  int max=-1;
  int i =0;
  //read and compute max size
  int offset=0;
  for(i=0;i<arguments.nb_runs;i++){
    while (fscanf(fin, "%d\n" , &sizes[m+offset]) != EOF  && offset<MAX_LINES) {
      if(arguments.max_size==0 || (sizes[m+offset] < arguments.max_size && sizes[m+offset]>= arguments.min_size)){
        if(sizes[m+offset]>max)max=sizes[m+offset];
        offset++;
      }
    }
    m+=offset;
    offset=0;
    rewind(fin);
  }
  fclose(fin);

  // Shuffling the array, using https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
  // Need to use the same seed on both nodes
  srand(42);
  for(i = m-1; i > 0 ; i--) {
    int j = randint(i+1);
    int tmp = sizes[i];
    sizes[i] = sizes[j];
    sizes[j] = tmp;
  }

  printf("[%d] m = %d, max=%d\n",get_rank(), m, max);
  // Build a totally stupid datatype
  struct { int a;int c; double b;int test[10][3];int tab[2][3];} value;
  MPI_Datatype mystruct;
  int          blocklens[4];
  MPI_Aint     indices[4];
  MPI_Datatype old_types[4], type2;
  MPI_Type_contiguous(3, MPI_INT, &type2);
  MPI_Type_commit(&type2);
  /* One value of each type, and two for the contiguous one */
  blocklens[0] = 1;
  blocklens[1] = 1;
  blocklens[2] = 2;
  blocklens[3] = 1;
  /* The base types */
  old_types[0] = MPI_INT;
  old_types[1] = MPI_DOUBLE;
  old_types[2] = type2;
  old_types[3] = MPI_UB;
  /* The locations of each element */
  MPI_Address( &value.a, &indices[0] );
  MPI_Address( &value.b, &indices[1] );
  MPI_Address( &value.tab, &indices[2] );
  MPI_Address( &value.a, &indices[3] );
  /* Make relative */
  indices[3] = sizeof(value);
  indices[2] = indices[2] - indices[0];
  indices[1] = indices[1] - indices[0];
  indices[0] = 0;
  MPI_Type_struct( 3, blocklens, indices, old_types, &mystruct );
  MPI_Type_commit( &mystruct );

    // initialize buffers for input and output (max size)

  /* my_send_buffer = (void*)malloc(max*sizeof(value));   */
  /* my_receive_buffer = (void*)malloc(max*sizeof(value));*/
   
  int max_buffer_size = max*2;
  if(max_buffer_size < 10000)
    max_buffer_size = 10000;
  my_send_buffer = (void*)malloc(max_buffer_size);
  my_receive_buffer = (void*)malloc(max_buffer_size);

  printf("[%d] Alloc size: %d \n", get_rank(), max_buffer_size);


  if(my_send_buffer==NULL || my_receive_buffer==NULL){
    printf("Can't allocate memory (%g GB), decrease max size of the messages \n",
          max_buffer_size/(1073741824));

    return -1;
  }

  //Init time
  base_time=get_time();

  //First Test : The Receive
  for(i=0;i<m;i++){
    test_op(output_files, "Recv", sizes[i], NB_RUNS);
  }


  //Second Test : The Isend
  for(i=0;i<m;i++){
    test_op(output_files, "Isend", sizes[i], NB_RUNS);
  }

  //Third Test : The Ping Pong
  for(i=0;i<m;i++){
    test_op(output_files, "PingPong", sizes[i], NB_RUNS);
  }

  test_op(output_files, "Wtime", 0, 10000000);

  for(i=1; i<=10000; i*=10)
    test_op(output_files, "Iprobe", i, NB_RUNS);

  for(i=1; i<=10000; i*=10)
    test_op(output_files, "Test", i, NB_RUNS);

    //second test is a test with the other datatype, to check if we have differences due to cache usage and buffering times
/*    MPI_Aint  struct_size;*/
/*    MPI_Type_extent(mystruct, &struct_size);*/
/*    for(i=0;i<m;i++){*/
/*      printf("%d : receive with size %d number of elts %d for struct of size %d\n", my_rank, sizes[i], sizes[i]/struct_size, struct_size);*/
/*      get_Recv(sizes[i], mystruct, 0, NB_RUNS);*/
/*    }*/
/*    */
/*    */

  MPI_Finalize();

  for(int i = 0; names[i]; i++) {
    fflush(output_files[i]);
    fclose(output_files[i]);
  }
  return 0;
}
