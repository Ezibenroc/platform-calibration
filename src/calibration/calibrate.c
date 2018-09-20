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
#include "utils.h"

#define MAX_LINES 2000
#define NB_RUNS 10
#define MAX_NAME_SIZE 256

static void * my_send_buffer;
static void * my_receive_buffer;

FILE* active_file = NULL;
char* my_basename = "calibration";
char* dir_name  = ".";

unsigned long long base_time;

typedef struct {
  char* sizefile;
  int max_size;
  int min_size;
  char* prefix;
  char* dir_name;
} my_args;



static char doc[] = "Runs MPI point to point benchmarks to compute values used for SMPI calibration";


static struct argp_option options[] = {
  {"sizeFile", 's', "SIZEFILE", 0, "filename of the size list"},
  {"min_size", 'm', "MIN SIZE", 0, "Minimum size to process"},
  {"max_size", 'M', "MAX SIZE", 0, "Maximum size to process"},
  {"prefix", 'p', "PREFIX", 0, "prefix of the csv files"},
  {"dir_name", 'd', "dir_name", 0, "Name/path of the directory to save files into. No trailing slashes."},
  { 0 }
};

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
  case 'p':
    arguments->prefix = arg;
    break;
   case 's':
    arguments->sizefile = arg;
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

FILE *open_file(const char* name){
    char* filename= malloc(MAX_NAME_SIZE*sizeof(char));
    sprintf(filename, "%s/%s_%s.csv", dir_name, my_basename, name);
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


static const char *names[] = {"Recv", "Isend", "PingPong", "Wtime", "Iprobe", "Test", NULL};
static const void (*functions[])(FILE*, int, int, unsigned long long) = {get_Recv, get_Isend, get_PingPong, get_Wtime,
                                                 get_Iprobe, get_Test};

void test_op(FILE *output_files[], int op_id, int size, int nb_runs, unsigned long long base_time) {
  functions[op_id](output_files[op_id], size, nb_runs, base_time);
}

int main(int argc, char** argv){

  bzero (&arguments, sizeof(my_args));


  if (argp_parse (&argp, argc, argv, 0, 0, &arguments) == ARGP_KEY_ERROR){
    fprintf(stderr,"error during the parsing of parameters\n");
    return 1;
  }

  if(arguments.prefix!=NULL)
    my_basename=arguments.prefix;
  if(arguments.dir_name!=NULL) {
    dir_name=arguments.dir_name;
  }
  if(arguments.sizefile==NULL){
    printf("Please provide a name for a file containing a list of sizes\n");
    return -1;
  }
  //
// load sizes from file
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

  int max=-1;
  int i =0;
  int nb_exp, largest_size;

  experiment_t *experiments = parse_experiment_file(names, arguments.sizefile, &nb_exp, &largest_size,
                                                    arguments.min_size, arguments.max_size);

  printf("[%d] nb_exp=%d, largest_size=%d\n",get_rank(), nb_exp, largest_size);
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
   
  int max_buffer_size = largest_size*2;
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
  for(i = 0; i < nb_exp; i++) {
    test_op(output_files, experiments[i].op_id, experiments[i].size, NB_RUNS, base_time);
  }

  MPI_Finalize();

  for(int i = 0; names[i]; i++) {
    fflush(output_files[i]);
    fclose(output_files[i]);
  }
  free(experiments);
  return 0;
}
