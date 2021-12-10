#!/bin/bash 
 
#PBS -q dssc
#PBS -l nodes=2:ppn=24
#PBS -l walltime=0:20:00 
 
cd $PBS_O_WORKDIR 
module load openmpi/4.0.3/gnu/9.3.0 


print_input(){

    nx=$1
    ny=$2
    nz=$3
    
    echo $((700*nx)), $nx, t > input.in
    echo $((700*ny)), $ny, t >> input.in
    echo $((700*nz)), $nz, t >> input.in

}

call_mpi(){
    nx=$1
    ny=$2
    nz=$3
    ntot=$((nx*ny*nz))
    nodetype="thin"
    maptype="node"
    print_input $nx $ny $nz

    /usr/bin/time -f "user : %U system: %S elapsed: %e CPU: %P CMD: %C \n"  mpirun --mca btl ^openib --map-by $maptype -np $ntot ./jacoby3D.x < input.in  >> ${nodetype}_${maptype}_${ntot}_${nx}_${ny}_${nz}.out 2>>error${nodetype}_${maptype}_${ntot}_${nx}_${ny}_${nz}.out


}

# call_mpi 1 1 1 
# call_mpi 4 1 1 
# call_mpi 2 2 1 
# call_mpi 8 1 1 
# call_mpi 4 2 1 
# call_mpi 2 2 2 
call_mpi 12 1 1 
call_mpi 4 3 1 
call_mpi 3 2 2 
call_mpi 6 2 1 
call_mpi 24 1 1 
call_mpi 12 2 1 
call_mpi 8 3 1 
call_mpi 6 4 1 
call_mpi 6 2 2 
call_mpi 4 3 2 
call_mpi 48 1 1 
call_mpi 24 2 1 
call_mpi 12 4 1 
call_mpi 12 2 2 
call_mpi 8 6 1 
call_mpi 6 4 2 

   
 
