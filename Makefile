CC=g++
FLAGS=-std=c++11 -fopenmp -lgomp -ggdb -Wall -Wextra

all: sequencial parallel
sequencial: life3d.cpp
	$(CC) $(FLAGS) life3d.cpp -o life3d

parallel: life3d-omp.cpp
	$(CC) $(FLAGS) life3d-omp.cpp -o life3d-omp

profile: life3d-omp.cpp
	kinst-ompp $(CC) $(FLAGS) life3d-omp.cpp -o life3d-omp

clean_profile:
	rm opari.rc opari.tab.c opari.tab.o life3d-omp.cpp.opari.inc life3d-omp.mod.cpp

clean:
	rm life3d life3d-omp

