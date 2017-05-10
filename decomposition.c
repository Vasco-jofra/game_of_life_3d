#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1)-BLOCK_LOW(id))
#define BLOCK_LOW(id,p,n) ((i)*(n)/(p))

int main(int argc, char **argv) {

    int blocksize[5];
	 int id,p;
  
	 MPI_Init (&argc, &argv);
	 MPI_Barrier(MPI_COMM_WORLD);
	 
	 MPI_Comm_rank (MPI_COMM_WORLD, &id);
	 MPI_Comm_size (MPI_COMM_WORLD, &p);
   
	 for(int u=0; u < p;u++)
		 blocksize[u] = BLOCK_SIZE(id,p,5);
	 		 printf("COLUNA:%d para P:%d\n",  blocksize[u],u);
  
    MPI_Finalize();

    return 0;
}