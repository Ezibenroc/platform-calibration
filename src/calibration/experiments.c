#include "experiments.h"

MPI_Request *req_array;
#define tag 1
#define datatype MPI_CHAR

void get_PingPong(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  MPI_Status status;
  unsigned long long start_time, send_time, recv_time;
  int i;
    
    
  MPI_Barrier(get_comm());
  if( get_rank() == 0 ) {
    
    for (i=0; i<nb_it; i++) {
      start_time=get_time();
      MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
      send_time = get_time();
      MPI_Recv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &status);
      recv_time = get_time()- send_time;
      print_in_file(file, "MPI_Send",count, start_time-base_time, send_time - start_time);
      print_in_file(file, "MPI_Recv",count, start_time-base_time, recv_time );
    }
  } else {
      for (i=0; i<nb_it; i++) {
        MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);
        MPI_Send(get_send_buffer(), count, datatype, 0, tag, get_comm());
      }

  }
}

void get_Isend(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  MPI_Status status;
  MPI_Request req;
  int i;
  unsigned long long start_time, total_time;
  
  if( get_rank() == 0 ) {
    for (i=0; i<nb_it; i++) {
      start_time=get_time();
      MPI_Isend(get_send_buffer(), count, datatype, 1, tag, get_comm(), &req);
      total_time=get_time()-start_time;
      print_in_file(file, "MPI_Isend", count, start_time-base_time, total_time);
      MPI_Recv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &status);
    }
  } else {
      for (i=0; i<nb_it; i++) {
        MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);
        MPI_Isend(get_send_buffer(), count, datatype, 0, tag, get_comm(), &req);
      }
  }
}

/*receive time (with waiting to avoid waiting times from late senders*/

void get_Recv(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  MPI_Status status;
  int i, flag, j;
  unsigned long long start_time, sleeptime, total_time;
  
  if( get_rank() == 0 ) {
    start_time=get_time();
    //first pingpong is just a test to compute how much we want to sleep to be sure the message arrived
    //do it with isend irecv to generate different events which won't pollute data exploitation
    MPI_Request r, r2;
    MPI_Isend(get_send_buffer(), count, datatype, 1, tag, get_comm(), &r);
    MPI_Wait(&r, &status);
    MPI_Irecv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &r2);
    MPI_Wait(&r2, &status);
    sleeptime = get_time() - start_time;    
    
    for (i=0; i<nb_it; i++) {
      flag=0;
      MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
      my_sleep(sleeptime);
      start_time=get_time();
      MPI_Recv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &status);
      total_time = get_time() - start_time;
      print_in_file(file, "MPI_Recv", count, start_time-base_time, total_time);
    }

    MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
    
    
  } else {
  
    MPI_Request r, r2;
    MPI_Irecv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &r2);
    MPI_Wait(&r2, &status);
    MPI_Isend(get_send_buffer(), count, datatype, 0, tag, get_comm(), &r);
    MPI_Wait(&r, &status);

    MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);

    for (i=0; i<nb_it; i++) {
    flag=0;
      MPI_Send(get_send_buffer(), count, datatype, 0, tag, get_comm());
      MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);
    }

  }
}


/*iProbe time*/


void get_Iprobe(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  MPI_Status status;
  int i, flag;
  unsigned long long start_time, sleeptime, total_time;
  
  if( get_rank() == 0 ) {

    for (i=0; i<nb_it; i++) {
      MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
      do {
        start_time=get_time();
        MPI_Iprobe(1, tag, get_comm(), &flag, &status);
        total_time = get_time() - start_time;
        print_in_file(file, "MPI_Iprobe", count, start_time-base_time, total_time);
      } while( !flag);

      MPI_Recv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &status);
    }
    MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
  } else {
    MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);

    for (i=0; i<nb_it; i++) {
      MPI_Send(get_send_buffer(), count, datatype, 0, tag, get_comm());
      do { /* ??? Change! add iprobe for rank 1 to make time comparable to that of rank 0 ! */
        MPI_Iprobe(0, tag, get_comm(), &flag, &status);
      } while( !flag);

      MPI_Recv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &status);

    }

  }
}


void get_Wtime(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  long i=0;
  unsigned long long start_time, sleeptime, total_time;
 
  //measure the time taken to perform n calls to MPI_Wtime
  start_time=get_time();
  for(i=0;i<nb_it;i++)
    {
      MPI_Wtime() ;
    }
    total_time = get_time() - start_time;
    print_in_file(file, "MPI_Wtime", count, start_time-base_time, total_time);
  /*
  double t1 = MPI_Wtime();
  while (MPI_Wtime() - t1 < nb_it) i++;
*/
}


void get_Test(FILE *file, int count, int nb_it, unsigned long long base_time)
{
  MPI_Status status;
  MPI_Request req;
  int i;
  int flag=0;
  unsigned long long start_time, sleeptime, total_time;
  
  if( get_rank() == 0 ) {
  
    for (i=0; i<nb_it; i++) {
      MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
      MPI_Irecv(get_recv_buffer(), count, datatype, 1, tag, get_comm(), &req);
      do {
            start_time=get_time();
		    MPI_Test( &req, &flag, &status );
		    total_time = get_time() - start_time;
            print_in_file(file, "MPI_Test", count, start_time-base_time, total_time);
	    } while (!flag);
    }
    MPI_Send(get_send_buffer(), count, datatype, 1, tag, get_comm());
  } else {
    MPI_Irecv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &req);
    MPI_Wait(&req, &status);
    for (i=0; i<nb_it; i++) {
      MPI_Send(get_send_buffer(), count, datatype, 0, tag, get_comm());
      MPI_Irecv(get_recv_buffer(), count, datatype, 0, tag, get_comm(), &req);
    do {		
		  MPI_Test( &req, &flag, &status );
	  } while (!flag);
    }
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
}
