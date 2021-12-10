#!/bin/bash 
 
#PBS -q dssc 
#PBS -l nodes=2:ppn=12
#PBS -l walltime=0:05:00 
 


module load openmpi/4.0.3/gnu/9.3.0 

cd $PBS_O_WORKDIR 

pwd
make clean
make csv



echo "#SIZE,MEAN,MEAN_MESSAGE,STD" >node.txt
echo "#SIZE,MEAN,MEAN_MESSAGE,STD" >socket.txt
echo "#SIZE,MEAN,MEAN_MESSAGE,STD" >core.txt

for i in  {2..24..1} 
do 
    mpirun --map-by core --mca btl ^openib -np $i ./ring.x >>core.txt
    mpirun --map-by socket  --mca btl ^openib -np $i ./ring.x >>socket.txt
    mpirun --map-by node --mca btl ^openib -np $i ./ring.x >>node.txt
done 
 
