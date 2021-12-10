#!/bin/bash 
 
#PBS -q dssc 
#PBS -l nodes=1:ppn=24
#PBS -l walltime=2:00:00 
 


module load openmpi/4.0.3/gnu/9.3.0 

cd $PBS_O_WORKDIR 

make default

for i in  1 4 8 12 16 20 24
do 
    /usr/bin/time -f "user: %U system: %S elapsed: %e CPU: %P CMD: %C \n"  mpirun --map-by core --mca btl ^openib -np $i ./matrix.x 2400 1000 700 0 0 0 >> 1_core.txt 2>>1_core.txt
    /usr/bin/time -f "user: %U system: %S elapsed: %e CPU: %P CMD: %C \n"  mpirun --map-by socket  --mca btl ^openib -np $i ./matrix.x 2400 1000 700 0 0 0 >> 1_socket.txt 2>>1_socket.txt
    /usr/bin/time -f "user: %U system: %S elapsed: %e CPU: %P CMD: %C \n"  mpirun --map-by core --mca btl ^openib -np $i ./matrix.x 1200 2000 700 0 0 0 >> 2_core.txt 2>>2_core.txt
    /usr/bin/time -f "user: %U system: %S elapsed: %e CPU: %P CMD: %C \n"  mpirun --map-by socket  --mca btl ^openib -np $i ./matrix.x 1200 2000 700 0 0 0 >> 2_socket.txt 2>>2_socket.txt


done
