g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s50e5k.in [0m > tests/our_out/s50e5k.300.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s50e5k.in: 
init_time: 0.002503 
proc_time: 10.340986

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s150e10k.in [0m > tests/our_out/s150e10k.1000.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s150e10k.in: 
init_time: 0.003457 
proc_time: 0.762804

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s200e50k.in [0m > tests/our_out/s200e50k.1000.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s200e50k.in: 
init_time: 0.052498 
proc_time: 1.702985

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s500e300k.in [0m > tests/our_out/s500e300k.2000.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s500e300k.in: 
init_time: 0.205451 
proc_time: 13.757289

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s5e50.in [0m > tests/our_out/s5e50.10.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s5e50.in: 
init_time: 0.000038 
proc_time: 0.000521

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s20e400.in [0m > tests/our_out/s20e400.500.our_out (2 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s20e400.in: 
init_time: 0.000148 
proc_time: 0.584055

