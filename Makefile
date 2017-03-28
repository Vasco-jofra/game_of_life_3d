CC=g++
FLAGS=-fopenmp -lgomp -g -Wall -Wextra

all: sequencial parallel
sequencial: life3d.cpp
	$(CC) $(FLAGS) life3d.cpp -o life3d

parallel: life3d-omp.cpp
	$(CC) $(FLAGS) life3d-omp.cpp -o life3d-omp

clean:
	rm life3d

