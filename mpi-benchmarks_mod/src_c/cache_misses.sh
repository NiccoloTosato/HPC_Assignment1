#!/bin/bash 
 
EXE="./IMB-MPI1"
make clean 
module load openmpi/4.0.3/gnu/9.3.0 
make -j2

    mpirun --map-by node -np 2 ${EXE} PingPong -msglog 28  2>node_ib_cache_misses.out 1>/dev/null
    mpirun --map-by node --mca pml ucx -mca btl ^uct -x UCX_NET_DEVICES=ib0 -np 2 ${EXE} PingPong -msglog 28  2> node_ucx_ib0_cache_misses.out 1>/dev/null
    mpirun --map-by node --mca pml ucx -mca btl ^uct -x UCX_NET_DEVICES=br0 -np 2 ${EXE} PingPong -msglog 28  2> node_ucx_br0_cache_misses.out 1>/dev/null
    mpirun --map-by node --mca pml ucx -mca btl ^uct -x UCX_NET_DEVICES=mlx5_0:1 -np 2 ${EXE} PingPong -msglog 28  2> node_ucx_mlx5_cache_misses.out 1>/dev/null
    mpirun --map-by node --mca pml ob1 --mca btl self,tcp -np 2 ${EXE} PingPong -msglog 28  2> node_ob1_selftcp_cache_misses.out 1>/dev/null
 
    mpirun --map-by socket -np 2 ${EXE} PingPong -msglog 28  2> socket_ib_cache_misses.out 1>/dev/null
    mpirun --map-by socket --mca pml ob1 --mca btl self,vader -np 2 ${EXE} PingPong -msglog 28  2> socket_ob1_selfvader_cache_misses.out 1>/dev/null
    mpirun --map-by socket --mca pml ob1 --mca btl self,tcp -np 2 ${EXE} PingPong -msglog 28  2> socket_ob1_selftcp_cache_misses.out 1>/dev/null
 
    mpirun --map-by core -np 2 ${EXE} PingPong -msglog 28  2> core_ib_cache_misses.out 1>/dev/null
    mpirun --map-by core --mca pml ob1 --mca btl self,vader -np 2 ${EXE} PingPong -msglog 28  2> core_ob1_selfvader_cache_misses.out 1>/dev/null
    mpirun --map-by core --mca pml ob1 --mca btl self,tcp -np 2 ${EXE} PingPong -msglog 28  2> core_ob1_selftcp_cache_misses.out 1>/dev/null

