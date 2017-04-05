g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s50e5k.in [0m > tests/our_out/s50e5k.300.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s50e5k.in: 
init_time: 0.001635 
proc_time: 5.634440

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s150e10k.in [0m > tests/our_out/s150e10k.1000.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s150e10k.in: 
init_time: 0.008958 
proc_time: 0.433545

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s200e50k.in [0m > tests/our_out/s200e50k.1000.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s200e50k.in: 
init_time: 0.018010 
proc_time: 0.901314

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s500e300k.in [0m > tests/our_out/s500e300k.2000.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s500e300k.in: 
init_time: 0.116892 
proc_time: 7.053284

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s5e50.in [0m > tests/our_out/s5e50.10.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s5e50.in: 
init_time: 0.000062 
proc_time: 0.000590

[1;38;5;215mRunning [32mlife3d-omp [1;38;5;215mtests/s20e400.in [0m > tests/our_out/s20e400.500.our_out (4 threads)  [ [1;38;5;41mSucceded[0m ]
OMP tests/s20e400.in: 
init_time: 0.000148 
proc_time: 0.323873

