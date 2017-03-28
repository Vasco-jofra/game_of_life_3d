CC=g++
FLAGS=-fopenmp -lgomp -g -Wall -Wextra

all: compile
compile: life3d.cpp
	$(CC) $(FLAGS) life3d.cpp -o life3d

clean:
	rm life3d

