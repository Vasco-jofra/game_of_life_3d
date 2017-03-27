CC=g++
FLAGS=-O3 -Wall -Wextra

all: compile
compile: life3d.cpp
	$(CC) $(FLAGS) life3d.cpp -o life3d

clean:
	rm project merged_project.c

