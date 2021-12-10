//#include <mpi.h>
#include "/usr/lib/x86_64-linux-gnu/openmpi/include/mpi.h" //extended path emacs is unable to find the correct header, very strange, to be checked
#include <stdio.h>  //printf stuff and other
#include <stdlib.h> //need for memory management
#include <string.h> //need for memcopy
#include <unistd.h> // need for sleep()

#define TRUE 1
#define FALSE 0
#define TYPE int
#define M_TYPE MPI_INT

int get_elem_count(int *matrix_shape) {
  return matrix_shape[0] * matrix_shape[1] * matrix_shape[2];
}

int get_submatrix_absolute_position(int *matrix_shape, int *submatrix_index) {

  int row = matrix_shape[0];
  int col = matrix_shape[1];
  int plane_elem = row * col;


  return plane_elem * submatrix_index[2] + col * submatrix_index[0] + submatrix_index[1];
}

void print_3D_matrix(TYPE *M, int *matrix_shape) {
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
  for (int i = 0; i < elem_count; ++i)
    i[M] = i; // wtf ? ?? ? ?? ? ? pls dont do this AHAHAHAHAH
}
long TYPE get_sum(TYPE* m,int elem_count){
  long  TYPE acc=0;
  for(int i=0;i<elem_count;++i)
    acc+=m[i];
  return acc;
}

void swap_shape(int* shape){
  int tmp;
  tmp=shape[0];
  shape[0]=shape[2];
  shape[2]=tmp;
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
#ifdef PRINT1
    printf("matrix size = %d X %d X %d\n",matrix_shape[0],matrix_shape[1],matrix_shape[2]);
    printf("\nAbout domain decomposition:\nrow_decomposition = %d subdomain\n",topology_shape[0]);
    printf("col_decomposition = %d subdomain\n", topology_shape[1]);
    printf("dept_decomposition = %d subdomain\n", topology_shape[2]);
#endif
    
    //allocate A,B
    A = malloc(sizeof(TYPE) * elem_count);
    B = malloc(sizeof(TYPE) * elem_count);

    
    init_matrix(A, elem_count);
    init_matrix(B, elem_count);
    sum_control=get_sum(A,elem_count);
    sum_control+=get_sum(B, elem_count);
    
#ifdef PRINT2
    printf("\nA MATRIX %d\n",elem_count);
    print_3D_matrix(B,matrix_shape);
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


  double time=0;
  if (rank == 0) {
    time=MPI_Wtime();
    //allocate 2 auxiliary matrix for matrix unrolling
    for (int i = 1; i < size; ++i) {
      get_submatrix_shape_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_shape);
      get_submatrix_index_by_rank(i, new_comm, matrix_shape, topology_shape,
                                  submatrix_index);
      my_elem_count = get_elem_count(submatrix_shape);

      MPI_Datatype submatrix_data_type;
      int abs_index=get_submatrix_absolute_position(matrix_shape, submatrix_index);
      /* submatrix_index[0]=0; */
      /* submatrix_index[1]=0; */
      /* submatrix_index[2]=0; */
      swap_shape(submatrix_shape);
      swap_shape(submatrix_index);
      swap_shape(matrix_shape);
      MPI_Type_create_subarray(3, matrix_shape, submatrix_shape, submatrix_index, MPI_ORDER_C, M_TYPE, &submatrix_data_type);
      MPI_Type_commit(&submatrix_data_type);
      MPI_Send(A, 1, submatrix_data_type, i, 0, new_comm);
      MPI_Send(B, 1, submatrix_data_type, i, 0, new_comm);
      MPI_Type_free(&submatrix_data_type);
#ifdef PRINT1
      printf("\nI m %d, at position (%d,%d,%d), ROW %d | COL %d | DEPT %d |TOTAL ELEM= %d \n",i,submatrix_index[0],submatrix_index[1],submatrix_index[2],submatrix_shape[0],submatrix_shape[1],submatrix_shape[2],my_elem_count);
#endif
            
    }
    time=MPI_Wtime()-time;
    //printf("Preparing overhead + sendtime  $%f\n",time);


  } else {

    time=MPI_Wtime();
    MPI_Status stat;
    get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape, submatrix_shape);
    get_submatrix_index_by_rank(rank, new_comm, matrix_shape, topology_shape, submatrix_index);
    MPI_Datatype submatrix_data_type;
    int my_el=get_elem_count(submatrix_shape);
    /* submatrix_index[0]=0; */
    /* submatrix_index[1]=0; */
    /* submatrix_index[2]=0; */
    swap_shape(submatrix_shape);
    swap_shape(matrix_shape);
    swap_shape(submatrix_index);
    /* MPI_Type_create_subarray(3, submatrix_shape, submatrix_shape, submatrix_index, MPI_ORDER_C, MPI_INT, &submatrix_data_type); */
    /* MPI_Type_commit(&submatrix_data_type); */
    
    //MPI_Recv(sA, 1, submatrix_data_type, MPI_ANY_SOURCE, MPI_ANY_TAG, new_comm, &stat);
    //MPI_Recv(sB, 1, submatrix_data_type, MPI_ANY_SOURCE, MPI_ANY_TAG, new_comm, &stat);
    MPI_Recv(sA, my_el, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, new_comm, &stat);
    MPI_Recv(sB, my_el, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, new_comm, &stat);
    
    /* MPI_Type_free(&submatrix_data_type); */
    time=MPI_Wtime()-time;
    //printf("Slave recv time $%f\n",time);
    usleep(rank*1000);
    print_3D_matrix(sA, submatrix_shape);

  }
  
  TYPE* C=NULL;
  time=0;
  if(rank==0) {
    get_submatrix_shape_by_rank(0, new_comm, matrix_shape,topology_shape, submatrix_shape);
    int plane_elem=matrix_shape[0]*matrix_shape[1];
    int plane_elem_sub=submatrix_shape[0]*submatrix_shape[1];
    
    C=malloc(elem_count*sizeof(TYPE));
    for(int i=0;i<submatrix_shape[2];++i)
      for(int j=0;j<submatrix_shape[0];++j)
	for(int k=0;k<submatrix_shape[1];++k)
	  C[i*plane_elem+j*matrix_shape[1]+k]=A[i*plane_elem+j*matrix_shape[1]+k]+B[i*plane_elem+j*matrix_shape[1]+k]; 
  }
  else{
    get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape, submatrix_shape);
    for(int i=0;i<get_elem_count(submatrix_shape);++i)
      sA[i]+=sB[i];
    //sA[i]=rank;
  }
  time=MPI_Wtime()-time;
  printf("Computation time $%f\n",time);
  
  //free(sB);

    
#ifdef PRINT2
    if(rank != 0){
    usleep(rank * 10000);
    printf("\n\nI'm %d and i will get\n", rank);
    print_3D_matrix(sA, submatrix_shape);
    }
#endif
    
    
    if(rank==0) {
      
      time=0;
      for(int i=1;i<size;++i){
	
	get_submatrix_shape_by_rank(i, new_comm, matrix_shape, topology_shape,
				    submatrix_shape);
	get_submatrix_index_by_rank(i, new_comm, matrix_shape, topology_shape,
				    submatrix_index);
	my_elem_count = get_elem_count(submatrix_shape);
	MPI_Datatype submatrix_data_type;
	MPI_Type_create_subarray(3, matrix_shape, submatrix_shape, submatrix_index, MPI_ORDER_C, M_TYPE, &submatrix_data_type);
	MPI_Type_commit(&submatrix_data_type);
	MPI_Status stat;
	MPI_Recv(C,1,submatrix_data_type,i,MPI_ANY_TAG,new_comm,&stat);
	
	printf("HO RICEVUTO DA %d\n",i);
	
	//print_3D_matrix(C, matrix_shape);fflush(stdout);
	MPI_Type_free(&submatrix_data_type);
	
      }
      time=MPI_Wtime()-time;
      printf("Gather time $%f\n", time);
    }else {
	get_submatrix_shape_by_rank(rank, new_comm, matrix_shape, topology_shape,
				    submatrix_shape);
	get_submatrix_index_by_rank(rank, new_comm, matrix_shape, topology_shape,
				    submatrix_index);
	my_elem_count = get_elem_count(submatrix_shape);
	submatrix_index[0]=0;
	submatrix_index[1]=0;
	submatrix_index[2]=0;
	
	MPI_Datatype submatrix_data_type;
	MPI_Type_create_subarray(3, submatrix_shape, submatrix_shape, submatrix_index, MPI_ORDER_C, M_TYPE, &submatrix_data_type);
	MPI_Type_commit(&submatrix_data_type);
	MPI_Send(sA,1,submatrix_data_type,0,0,new_comm);
	MPI_Type_free(&submatrix_data_type);
	
    }
    //free(sA);
  
  
  if(rank==0) {
    
#ifdef PRINT2
  print_3D_matrix(C, matrix_shape);
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
