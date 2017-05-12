CC=g++
MPI_CC=mpic++
FLAGS=-std=c++11 -ggdb -Wall -Wextra
OMP_FLAGS=-fopenmp -lgomp

all: sequencial parallel

sequencial: life3d.cpp
	$(CC) $(OMP_FLAGS) $(FLAGS) life3d.cpp -o life3d

parallel: life3d-omp.cpp
	$(CC) $(OMP_FLAGS) $(FLAGS) life3d-omp.cpp -o life3d-omp

mpi: life3d-omp.cpp
	$(MPI_CC) $(FLAGS) life3d-mpi.cpp -o life3d-mpi

profile: life3d-omp.cpp
	kinst-ompp $(CC) $(FLAGS) life3d-omp.cpp -o life3d-omp

clean_profile:
	rm opari.rc opari.tab.c opari.tab.o life3d-omp.cpp.opari.inc life3d-omp.mod.cpp

clean:
	rm life3d life3d-omp

