#!/bin/bash 
 
#PBS -q dssc 
#PBS -l nodes=2:ppn=2 
#PBS -l walltime=0:20:00 
 

cd /u/dssc/s271550/fast/mpi-benchmarks/src_c 

make clean 
module load intel 
make 

EXE="/u/dssc/s271550/fast/mpi-benchmarks/src_c/IMB-MPI1"

cd $PBS_O_WORKDIR 
 
for i in 1 2 3 4 5 6 7 8 9 10; do 
    mpirun -f $PBS_NODEFILE ${EXE} PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> node_ib_intel.out 
done 
 
for i in 1 2 3 4 5 6 7 8 9 10; do 
    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,1 ${EXE} PingPong >> socket_ib_intel.out
done 
 
for i in 1 2 3 4 5 6 7 8 9 10; do 
    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,2 ${EXE} PingPong >> core_ib_intel.out
done 
 
 
cat $PBS_NODEFILE >> thin_resources_used_intel.out 
 
