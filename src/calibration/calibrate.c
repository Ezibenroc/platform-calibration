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
#include <libxml/xmlreader.h>


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
  {"filename", 'f', "FILENAME", 0, "XML filename"},
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


static int parse_xml_file(const char* filename, my_args *arguments){
  xmlTextReaderPtr reader;
  int ret;
  reader = xmlReaderForFile(filename, NULL, 0);
  if (reader != NULL) {
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
      const xmlChar *name, *value;

      name = xmlTextReaderConstName(reader);
      if (name == NULL)
        name = BAD_CAST "--";
      value = xmlTextReaderGetAttribute(reader, "value");

       if(!strcmp(name, "iterations"))
         arguments->nb_runs=atoi(value);
       if(!strcmp(name, "minSize"))
         arguments->min_size=atoi(value);
       if(!strcmp(name, "maxSize"))
         arguments->max_size=atoi(value);
       if(!strcmp(name, "prefix"))
         arguments->prefix=(char*)value;
       if(!strcmp(name, "dirname"))
         arguments->dir_name=(char*)value;
       if(!strcmp(name, "sizeFile"))
         arguments->sizefile=(char*)value;
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if (ret != 0) {
      fprintf(stderr, "%s : failed to parse\n", filename);
    }
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

void switch_file(char* name){
    int topo;
    MPI_Barrier(MPI_COMM_WORLD);
    //dump previous buffer
    if(active_file){
      fflush(active_file);
      fclose(active_file);
    }
    //open new file
    char* filename= malloc(MAX_NAME_SIZE*sizeof(char));
    sprintf(filename, "%s/%s_%s.csv", dir_name, basename, name);
    active_file=fopen(filename, "w");
    MPI_Barrier(MPI_COMM_WORLD);
}

void print_in_file(const char* func, int count, unsigned long long start, unsigned long long resul){
  fprintf(active_file, "%s,%d,%f,%e\n", func, count, (start-base_time)/precision, resul/precision);
}




int main(int argc, char** argv){
  LIBXML_TEST_VERSION

  bzero (&arguments, sizeof(my_args));


  if (argp_parse (&argp, argc, argv, 0, 0, &arguments) == ARGP_KEY_ERROR){
    fprintf(stderr,"error during the parsing of parameters\n");
    return 1;
  }
  parse_xml_file(arguments.filename, &arguments);

  //override xml with command line ?
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
// load sizes from file
  int sizes[MAX_LINES*arguments.nb_runs];
  int index;
  int my_rank;
  int size;
  int m=0;

  MPI_Init(&argc, &argv);

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

  switch_file("Recv");
  //int i;
  for(i=0;i<m;i++){
    //printf("%d : receive with size %d\n", my_rank, sizes[i]);
    get_Recv(sizes[i], NB_RUNS);
  }


  //Second Test : The Isend
  switch_file("Isend");

  for(i=0;i<m;i++){
    //printf("%d : Isendtime with size %d\n", my_rank, sizes[i]);
    get_iSendtime(sizes[i], NB_RUNS);
  }

  //Third Test : The Ping Pong
  switch_file("PingPong");

  for(i=0;i<m;i++){
    //printf("%d : PingPong Test with size %d\n", my_rank, sizes[i]);
    get_PingPongtime(sizes[i], NB_RUNS);
  }

  //Fourth Test : MPI_Wtime
  switch_file("Wtime");

  get_Wtime(0, 10000000);

  //Fifth Test : MPI_Iprobe
  switch_file("Iprobe");

  for(i=1; i<=10000; i*=10)
    get_Iprobe(i, 10);

  //Sixth Test : MPI_Test
  switch_file("Test");
  for(i=1; i<=10000; i*=10)
    get_Test(i, 10);

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
  xmlCleanupParser();

return 0;
}
