# 1D ring
### General info
This code is delivered with a dedicated Makefile. This Makefile has several options.
All options compile the code at least with **-Wall** and **-Wextra** flags active.
Available rules:

- `` make default `` this rule provides a code that satisfy the assignment requests. No ``stdout`` messages.
- ``make debug `` in this way we compile the code in order to get the execution time for each process and a message with the output requested.
- ``make debug2`` provides an executable with all the previous message with additional messages about topology.
- ``make csv`` I used this rule to compile and time the program that provide suitable output appendable directly to csv.make
- ``make format`` code formatter provided by clang.
- ``make clean`` remove executable produced

The code is tested with **openMPI 4.03** and up to 96 precesses.
Must be runned as follow: ``mpirun -np #procs ring.x`` .

# 3D matrix sum
## 3D matrix with no topology
### General info
This code is delivered with a dedicated Makefile. All options compile the code at least with **-Wall** and **-Wextra** flags active.

Available rules:

- `` make default `` provide a program that satisfy the assigment requests, only message of correct result is provided from executable. ``-O3`` flag is specified.
- `` make timeit `` as previous but each process provide some time output.
- `` make print `` resulting code provide some output from each processor that explain how many elements they get and if the result is correct.
- ``make format`` code formatter provided by clang.
- ``make clean`` remove executable produced
The code is tested with **openMPI 4.03**.

How to run it: ``mpirun -np #nprocs matrix.x #nx #ny #nz``. Total element number must be greater or equalt than processors number. The element are equally distribuited to worker and reminder is assigned in round robin way.

## 3D matrix with topology
### General info
This code is delivered with a dedicated Makefile. All options compile the code at least with **-Wall** and **-Wextra** flags active.

Available rules:

- `` make default `` provide a program that satisfy the assigment requests, message of correct result is provided from executable. The output describe virtual topology shape, beacause it can be left unspecified by user. ``-O3`` flag is specified.
- `` make timeit `` as previous but each process provide some time output.
- `` make print1 `` resulting code provide some output from each processor that explain how many elements they get and subdomain shape. Correctness of results is provided.
- `` make print2 `` **note** use only with small matrices to understand domain decomposition and virtual topology. The executable print user friendly matrices and subdomain.
- ``make format`` code formatter provided by clang.
- ``make clean`` remove executable produced
The code is tested with **openMPI 4.03**.

How to run it: ``mpirun -np #nprocs matrix.x #nx #ny #nz #nprocsx #nprocsy #nprocsz``. Total element number must be greater or equalt than processors number. The element are equally distribuited to worker and reminder is assigned in round robin way. If #nprocs[x/y/z] is set to 0, MPI will provide topology shape on respective direction. 
