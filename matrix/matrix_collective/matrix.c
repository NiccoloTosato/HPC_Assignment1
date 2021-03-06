#include <mpi.h>
//#include "/usr/lib/x86_64-linux-gnu/openmpi/include/mpi.h" //extended path emacs is unable to find the correct header, very strange, to be checked
#include <stdio.h>  //printf stuff and other
#include <stdlib.h> //need for memory management
#include <string.h> //need for memcopy
#include <unistd.h> // need for sleep()

#define TRUE 1
#define FALSE 0
#define TYPE int
#define M_TYPE MPI_INT

int get_elem_count(int *matrix_shape) {
  /**
   get matrix elements count given a shape
   */
  
  return matrix_shape[0] * matrix_shape[1] * matrix_shape[2];
}

int get_submatrix_absolute_position(int *matrix_shape, int *submatrix_index) {
  /**
   get submatrix absolute index position, given matrix shape and submatrix index
   */
  
  int row = matrix_shape[0];
  int col = matrix_shape[1];
  int plane_elem = row * col;


  return plane_elem * submatrix_index[2] + col * submatrix_index[0] + submatrix_index[1];
}

void print_3D_matrix(TYPE *M, int *matrix_shape) {
  /**
     print 3d matrix in a user friendly way
   */
  
  int dept=matrix_shape[2];
  int col=matrix_shape[1];
  int row=matrix_shape[0];
  for (int k = 0; k < dept; ++k) {
    printf("\n---plane %d---\n", k);
    for (int i = 0; i < row; ++i) {
      for (int j = 0; j < col; ++j) {
        printf("%2.3d ", M[i * col + j + k * row * col]);
      }
      printf("\n");
    }
  }

}

void get_submatrix(TYPE *M, TYPE *sM, int *submatrix_shape, int *submatrix_index, int *matrix_shape) {
  /**
       extract a submatrix and unroll it in a contiguo memory region, given index, matrix shape and submatrix shape
   */

  int base_index =  get_submatrix_absolute_position(matrix_shape, submatrix_index);

  int submatrix_plane_elem = submatrix_shape[0] * submatrix_shape[1];
  int matrix_plane_elem = matrix_shape[0] * matrix_shape[1];
  int row_elem = submatrix_shape[1];

  for (int k = 0; k < submatrix_shape[2]; ++k) { // iterate on dept
    for (int j = 0; j < submatrix_shape[0];
         ++j) { // iterate on row,copy  colum lenght i.e submatrix_shape[1]
      memcpy(&sM[k * submatrix_plane_elem + row_elem * j], //dest
             &M[base_index + k * matrix_plane_elem + j * matrix_shape[1]], //source
             row_elem * sizeof(TYPE)); //elem count
    }
  }
}

void get_submatrix_shape_by_rank(int rank, MPI_Comm communicator, int *matrix_shape, int *topology_shape, int *submatrix_shape) {
  /**
     given a communicator, topology shape, matrix shape return by rank each submatrix shape
   */
  
  int coords[3] = {0};
  // get coords for rank in this topology
  MPI_Cart_coords(communicator, rank, 3, coords);

  // get matrix shape
  int row, col, dept;
  row = matrix_shape[0];
  col = matrix_shape[1];
  dept = matrix_shape[2];

  // get topology shape
  int row_cpu, col_cpu, dept_cpu;
  row_cpu = topology_shape[0];
  col_cpu = topology_shape[1];
  dept_cpu = topology_shape[2];

  // integer division!
  int my_col, my_row, my_dept;
  my_col = col / col_cpu;
  my_row = row / row_cpu;
  my_dept = dept / dept_cpu;

  // set reminder

  my_row = (coords[0] < row % row_cpu) ? my_row + 1 : my_row;
  my_col = (coords[1] < col % col_cpu) ? my_col + 1 : my_col;
  my_dept = (coords[2] < dept % dept_cpu) ? my_dept + 1 : my_dept;

  submatrix_shape[0] = my_row;
  submatrix_shape[1] = my_col;
  submatrix_shape[2] = my_dept;

  // printf("\n\tdebug rank=%d %d %d %d\n",rank,my_row,my_col,my_dept);
}

void get_submatrix_index_by_rank(int rank, MPI_Comm communicator, int *matrix_shape, int *topology_shape, int *submatrix_index) {
  /**
     given a communicator,topology shape, matrix shape return by rank each submatrix index
  */
  
  // get coordinate
  int coords[3];
  MPI_Cart_coords(communicator, rank, 3, coords);

  int total_elem[3] = {0, 0, 0};
  // sum on every direction
  int tmp_coords[3];

  MPI_Cart_coords(communicator, rank, 3, tmp_coords);
  for (int i = 0; i < coords[0]; i++) {
    int tmp_elem[3] = {0};
    int tmp_rank;
    tmp_coords[0] = i;
    MPI_Cart_rank(communicator, tmp_coords, &tmp_rank);
    get_submatrix_shape_by_rank(tmp_rank, communicator, matrix_shape, topology_shape, tmp_elem);
    total_elem[0] += tmp_elem[0];
  }

  MPI_Cart_coords(communicator, rank, 3, tmp_coords);
  for (int i = 0; i < coords[1]; i++) {
    int tmp_elem[3] = {0};
    int tmp_rank;
    tmp_coords[1] = i;
    MPI_Cart_rank(communicator, tmp_coords, &tmp_rank);
    get_submatrix_shape_by_rank(tmp_rank, communicator, matrix_shape, topology_shape, tmp_elem);
    total_elem[1] += tmp_elem[1];
  }

  MPI_Cart_coords(communicator, rank, 3, tmp_coords);
  for (int i = 0; i < coords[2]; i++) {
    int tmp_elem[3] = {0};
    int tmp_rank;
    tmp_coords[2] = i;
    MPI_Cart_rank(communicator, tmp_coords, &tmp_rank);
    get_submatrix_shape_by_rank(tmp_rank, communicator, matrix_shape,
                                topology_shape, tmp_elem);
    total_elem[2] += tmp_elem[2];
  }

  memcpy(submatrix_index, total_elem, sizeof(int) * 3);
  // you have the position!
}

void init_matrix(TYPE *M, int elem_count) {
  /**
       init a matrix given his length
   */
  
  for (int i = 0; i < elem_count; ++i)
    i[M] = i; // wtf ? ?? ? ?? ? ? pls dont do this AHAHAHAHAH
}
long TYPE get_sum(TYPE* m,int elem_count){
  /**
     reduce all matrix elements, for testing purpose
  */
  
  long  TYPE acc=0;
  for(int i=0;i<elem_count;++i)
    acc+=m[i];
  return acc;
}

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);
  int rank, size;
  int row_cpu, col_cpu, dept_cpu;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (argc < 7) {
    if (rank == 0)
      printf("No enought args\n");
    MPI_Finalize();
    return 1;
  }

  // save matrix shape into an array
  int matrix_shape[3];
  matrix_shape[0] = atoi(argv[1]);
  matrix_shape[1] = atoi(argv[2]);
  matrix_shape[2] = atoi(argv[3]);

  // read topology shape
  row_cpu = atoi(argv[4]);
  col_cpu = atoi(argv[5]);
  dept_cpu = atoi(argv[6]);
  
  // total matrix elements
  int elem_count = get_elem_count(matrix_shape);

  MPI_Comm new_comm;
  int ndims = 3, topology_shape[3] = {0}, period[3] = {FALSE};
  int coords[3];

  topology_shape[0] = row_cpu;
  topology_shape[1] = col_cpu;
  topology_shape[2] = dept_cpu;

  MPI_Dims_create(size, 3, topology_shape);

  // update the shape variable
  row_cpu = topology_shape[0];
  col_cpu = topology_shape[1];
  dept_cpu = topology_shape[2];

  MPI_Cart_create(MPI_COMM_WORLD, ndims, topology_shape, period, TRUE,
                  &new_comm);
  MPI_Cart_coords(new_comm, rank, 3, coords);

  //A B matrix pointer
  TYPE *A = NULL;
  TYPE *B = NULL;
  long TYPE sum_control=0;

  if (rank == 0) {
    printf("matrix size = %d X %d X %d\n",matrix_shape[0],matrix_shape[1],matrix_shape[2]);
    printf("\nAbout domain decomposition:\nrow_decomposition = %d subdomain\n",topology_shape[0]);
    printf("col_decomposition = %d subdomain\n", topology_shape[1]);
    printf("dept_decomposition = %d subdomain\n", topology_shape[2]);
    
    //allocate A,B
    A = malloc(sizeof(TYPE) * elem_count);
    B = malloc(sizeof(TYPE) * elem_count);


    init_matrix(A, elem_count);
    init_matrix(B, elem_count);
    sum_control=get_sum(A,elem_count);
    sum_control+=get_sum(B, elem_count);
    
#ifdef PRINT2
    printf("\nA MATRIX\n");
    print_3D_matrix(A,matrix_shape);
#endif

  }


  //all processes execute this chunk
  int submatrix_shape[3];
  int submatrix_index[3];
  int my_elem_count;
  
  get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape,
                              submatrix_shape);
  get_submatrix_index_by_rank(rank, new_comm, matrix_shape, topology_shape,
                              submatrix_index);
  my_elem_count = get_elem_count(submatrix_shape);

  //allocate local matrices
  TYPE *sA = malloc(sizeof(TYPE) * my_elem_count);
  TYPE *sB = malloc(sizeof(TYPE) * my_elem_count);

  // unroll matrix
  int *displacement = malloc(size * sizeof(int)); // move inside if,
  int *sendcount = malloc(size * sizeof(int));
  double time=0;
  if (rank == 0) {
    time=MPI_Wtime();
    int accumulator = 0;
    //allocate 2 auxiliary matrix for matrix unrolling
    TYPE *A_unrolled = malloc(sizeof(TYPE) * get_elem_count(matrix_shape));
    TYPE *B_unrolled = malloc(sizeof(TYPE) * get_elem_count(matrix_shape));

    for (int i = 0; i < size; ++i) {

      get_submatrix_shape_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_shape);
      get_submatrix_index_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_index);
      my_elem_count = get_elem_count(submatrix_shape);
      sendcount[i] = my_elem_count;
      displacement[i] = accumulator;
#ifdef PRINT1
      printf("\nI m %d, at position (%d,%d,%d), ROW %d | COL %d | DEPT %d |TOTAL ELEM= %d \n",i,submatrix_index[0],submatrix_index[1],submatrix_index[2],submatrix_shape[0],submatrix_shape[1],submatrix_shape[2],my_elem_count);
#endif


      //allocate submatrix
      TYPE *sMA = malloc(sizeof(TYPE) * my_elem_count);
      TYPE *sMB = malloc(sizeof(TYPE) * my_elem_count);
      
      get_submatrix(A, sMA, submatrix_shape, submatrix_index, matrix_shape);
      memcpy(&A_unrolled[accumulator], sMA, sizeof(TYPE) * my_elem_count);

      get_submatrix(B, sMB, submatrix_shape, submatrix_index, matrix_shape);
      memcpy(&B_unrolled[accumulator], sMB, sizeof(TYPE) * my_elem_count);

      accumulator += my_elem_count;

      free(sMA);
      free(sMB);
    }
    
    time=MPI_Wtime()-time;
    printf("Preparing overhead $%f\n",time);

    get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape, submatrix_shape);
    get_submatrix_index_by_rank(rank, new_comm, matrix_shape, topology_shape, submatrix_index);
    my_elem_count = get_elem_count(submatrix_shape);
    time=MPI_Wtime();
    MPI_Scatterv(A_unrolled, sendcount, displacement, M_TYPE, sA, my_elem_count, M_TYPE, 0, new_comm);
    MPI_Scatterv(B_unrolled, sendcount, displacement, M_TYPE, sB, my_elem_count, M_TYPE, 0, new_comm);
    time=MPI_Wtime()-time;
    printf("Master send time $%f\n",time);
    free(A_unrolled);
    free(B_unrolled);
  } else {

    get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape,
                                submatrix_shape);
    //scatter matrices
    time=MPI_Wtime();
    MPI_Scatterv(NULL, NULL, NULL, M_TYPE, sA, my_elem_count, M_TYPE, 0, new_comm);
    MPI_Scatterv(NULL, NULL, NULL, M_TYPE, sB, my_elem_count, M_TYPE, 0, new_comm);
    time=MPI_Wtime()-time;
    printf("Slave recv time $%f\n",time);
  }

#ifdef PRINT2
  usleep(rank * 10000);
  printf("\n\nI'm %d and i will get\n", rank);
  print_3D_matrix(sA, submatrix_shape);
#endif

  
  time=0;

  for(int i=0;i<my_elem_count;++i)
    sA[i]+=sB[i];

  time=MPI_Wtime()-time;
  printf("Computation time $%f\n",time);
  //free sB after use
  free(sB);

  
  
  TYPE* C=NULL;
  if(rank==0)
    C=malloc(elem_count*sizeof(TYPE));
  time=0;
  MPI_Gatherv(sA, my_elem_count, M_TYPE, C, sendcount, displacement, M_TYPE,0, new_comm);
  time=MPI_Wtime()-time;
  printf("Gather time $%f\n", time);
  free(sA);

  
  if (rank == 0) {
    time=MPI_Wtime();
    TYPE *mA = malloc(elem_count * sizeof(TYPE));
    // print_3D_matrix_shape(A, matrix_shape);
    int start_position = 0;
    for (int i = 0; i < size; ++i) {
      get_submatrix_shape_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_shape);
      get_submatrix_index_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_index);
      int absolute_position = get_submatrix_absolute_position(matrix_shape, submatrix_index);
      int plane_elem = matrix_shape[0] * matrix_shape[1];
      for (int k = 0; k < submatrix_shape[2]; ++k) {
        for (int j = 0; j < submatrix_shape[0]; ++j) {
          memcpy(&mA[absolute_position + j * matrix_shape[1] + k * plane_elem], //dest
                 &C[start_position + j * submatrix_shape[1]], //source
                 submatrix_shape[1] * sizeof(TYPE)); //elem count
        }
      }
      start_position += get_elem_count(submatrix_shape);
    }
      time=MPI_Wtime()-time;
      printf("Reorder time $%f\n",time);

#ifdef PRINT2
    print_3D_matrix(mA, matrix_shape);
#endif

    if(sum_control==get_sum(C, elem_count))
      printf("RISULTATO CORRETTO!\n");
    else
      printf("errore!\n");

    free(A);
    free(B);
    free(C);
  }

  MPI_Finalize();
}
