#include <mpi.h>
#include <stdio.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define NRUNS 100000
int main(int argc, char *argv[]) {

  int myrank, size;
  MPI_Comm New_comm;

  // broadcast arg to all process
  MPI_Init(&argc, &argv);

  // count how many processes are spawned
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // create a 1D closed ring
  int period[1] = {TRUE}; // set periodic
  int reorder = TRUE;     // allow MPI to reorder
  int ndims = 1;          // set 1, 1D ring !
  int dims[1] = {size};   // how many proc in each dim --> size

  MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, period, reorder, &New_comm);

  //get new rank among new communicator
  MPI_Comm_rank(New_comm, &myrank);


  //get the coords
  int coords[1];
  MPI_Cart_coords(New_comm, myrank, 1, coords);
  
#ifdef DEBUG2
  printf("DEBUG: I'm %d/%d at coords %d\n", myrank, size, coords[0]);
#endif
  //tag proportional to rank 
  int my_tag = myrank * 10;

  // get dest (right and left neighbour)
  int dest;
  int source;
  MPI_Cart_shift(New_comm, 1, 1, &source, &dest);

#ifdef DEBUG2
  printf("DEBUG: I'm %d and recv from %d and sent to %d\n", myrank, source, dest);
#endif

  //debclare request and status type in order to use non-blocking comm
  MPI_Request req[4];
  MPI_Status stat[4];
  double time=0;
  double time_mean=0;


  int message_counter;
  int rmessage_s,rmessage_r,lmessage_s,lmessage_r;

  for(int i=0;i<NRUNS;i++) {

  //init right and left messages sender buffer
    rmessage_s = -myrank;
    lmessage_s = +myrank;
  //init right and left messages receiver buffer
    rmessage_r = 0;
    lmessage_r = 0;
    message_counter=0;


    time=MPI_Wtime();
  //Initial iteraton
    MPI_Isend(&rmessage_s, 1, MPI_INT, dest, my_tag, New_comm, &req[0]);
    MPI_Isend(&lmessage_s, 1, MPI_INT, source, my_tag, New_comm, &req[2]);

    MPI_Irecv(&rmessage_r, 1, MPI_INT, source, MPI_ANY_TAG, New_comm, &req[1]);
    MPI_Irecv(&lmessage_r, 1, MPI_INT, dest, MPI_ANY_TAG, New_comm, &req[3]);

    MPI_Wait( &req[0], &stat[0]); // necessary in order to protect rmessage_s buffer
    MPI_Wait( &req[1], &stat[1]); // necessary in order to protect rmessage_r buffer
    MPI_Wait( &req[2], &stat[2]); // necessary in order to protect lmessage_s buffer
    MPI_Wait( &req[3], &stat[3]); // necessary in order to protect lmessage_r buffer
    message_counter++;
  //alternative way to wait!
    //MPI_Waitall(4, req, stat);

    while (1) {
    //prepare sender buffer
      rmessage_s = rmessage_r - myrank;
      lmessage_s = lmessage_r + myrank;

      MPI_Isend(&rmessage_s, 1, MPI_INT, dest, stat[1].MPI_TAG, New_comm, &req[0]); //send to right with the appropriate tag
      MPI_Isend(&lmessage_s, 1, MPI_INT, source, stat[3].MPI_TAG, New_comm, &req[2]); //send to left with the appropriate tag

      MPI_Irecv(&rmessage_r, 1, MPI_INT, source, MPI_ANY_TAG, New_comm, &req[1]); //receive from left (source)
      MPI_Irecv(&lmessage_r, 1, MPI_INT, dest, MPI_ANY_TAG, New_comm, &req[3]); //receive from right (dest)

      MPI_Wait( &req[0], &stat[0]); // necessary in order to protect rmessage_s buffer
      MPI_Wait( &req[1], &stat[1]); // necessary in order to protect rmessage_r buffer
      MPI_Wait( &req[2], &stat[2]); // necessary in order to protect lmessage_s buffer
      MPI_Wait( &req[3], &stat[3]); // necessary in order to protect lmessage_r buffer
      message_counter++;
    //alternative way to wait!
      //MPI_Waitall(4, req, stat);

      if ((stat[1].MPI_TAG == my_tag) && (stat[3].MPI_TAG == my_tag)) //stop condition, when i receive 2 message with the original tag ! 
	break;
    }

    time=MPI_Wtime()-time;
    time_mean+=time;

  }
  time_mean/=NRUNS;
#ifdef CSV
  if(myrank==0)
    printf("%d,%f,%f\n",size,(1e6)*time_mean,(1e6)*time_mean/(size));
#endif
#ifdef DEBUG
    printf("DEBUG: I'm %d My time %f uSec,mean time per message %f\n",myrank,(1e6)*time_mean,(1e6)*time_mean/(size));
#endif




  //write to output file
  char file_name[]="./out.txt";
  MPI_File my_file;

  //open file,if present and delete it!
  MPI_File_open( MPI_COMM_WORLD, file_name, MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &my_file );
  MPI_File_close(&my_file);
  MPI_Barrier(New_comm);
  //create new definitive
  MPI_File_open( MPI_COMM_WORLD, file_name, MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &my_file );
  
  
  char buffer[100];
  int n = sprintf(buffer,"I am process %d and I have received %d  messages. My final messages have tag %d and value %d, %d\n",myrank,message_counter,stat[1].MPI_TAG,rmessage_r,lmessage_r);
  
  MPI_File_write_ordered(my_file,buffer, n, MPI_CHARACTER, &stat[0]);
  MPI_File_close(&my_file);
  if(myrank==0)
    printf("Output placed in %s\n",file_name);
  
#ifdef DEBUG
  printf("DEBUG: I am process %d and I have received %d  messages. My final messages have tag %d and value %d, %d\n",myrank,message_counter,stat[1].MPI_TAG,rmessage_r,lmessage_r);
#endif
  
  MPI_Finalize();
  
}
