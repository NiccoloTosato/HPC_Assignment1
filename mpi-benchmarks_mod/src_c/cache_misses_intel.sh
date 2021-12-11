#!/bin/bash 
 
EXE="./IMB-MPI1"
make clean 
module load intel
make -j2


    mpirun -f $PBS_NODEFILE ${EXE} PingPong -msglog 28  2> intel_node_cache_misses.out 1>/dev/null
 
    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,1 ${EXE} PingPong -msglog 28  2> intel_socket_cache_misses.out 1>/dev/null

    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,2 ${EXE} PingPong -msglog 28  2> intel_core_cache_misses.out 1>/dev/null
