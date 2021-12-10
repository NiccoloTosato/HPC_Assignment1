#!/bin/bash 
 
#PBS -q dssc 
#PBS -l nodes=ct1pg-gnode003:ppn=2 
#PBS -l walltime=2:00:00 
 
#across socket 
#mpirun -ppn=1 -env I_MPI_DEBUG 5 -genv I_MPI_PIN_PROCESSOR_LIST 0,1 ./IMB-MPI1 PingPong msglog 28 
 
#across core 
cd /u/dssc/s271550/fast/mpi-benchmarks/src_c 
make clean 
module load intel 
make 
EXE="/u/dssc/s271550/fast/mpi-benchmarks/src_c/IMB-MPI1"
cd $PBS_O_WORKDIR 
 
#for i in 1 2 3 4 5 6 7 8 9 10; do 
#    mpirun -f $PBS_NODEFILE ${EXE} PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> node_ib_intel.out 
    #mpirun --map-by node --mca pml ob1 --mca btl self,tcp -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> node_ob1_selftcp_intel.out 
    #mpirun --map-by node --mca pml ob1 -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> node_ob1_intel.out 
#done 
 
for i in 1 2 3 4 5 6 7 8 9 10; do 
    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,1 ${EXE} PingPong -msglog 28 >> socket_ib_intel.out
    #mpirun --map-by socket -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> socket_ib_intel.out 
    #mpirun --map-by socket --mca pml ob1 --mca btl self,vader -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> socket_ob1_selfvader_intel.out 
    #mpirun --map-by socket --mca pml ob1 --mca btl self,tcp -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> socket_ob1_selftcp_intel.out 
    #mpirun --map-by socket --mca pml ob1 -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> socket_ob1_intel.out 
done 
 
for i in 1 2 3 4 5 6 7 8 9 10; do 
    mpirun -genv I_MPI_PIN_PROCESSOR_LIST 0,2 ${EXE} PingPong -msglog 28 >> core_ib_intel.out
    #mpirun --map-by core --mca pml ob1 --mca btl self,vader -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> core_ob1_selfvader_intel.out 
    #mpirun --map-by core --mca pml ob1 --mca btl self,tcp -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> core_ob1_selftcp_intel.out 
    #mpirun --map-by core --mca pml ob1 -np 2 ./mpi-benchmarks/src_c/IMB-MPI1 PingPong -msglog 28 | grep -v ^\# | grep -v '^$' | sed -r 's/^\s+//;s/\s+/,/g' >> core_ob1_intel.out 
 
done 
 
 
 
cat $PBS_NODEFILE >> thin_resources_used_intel.out 
 
rm task*
