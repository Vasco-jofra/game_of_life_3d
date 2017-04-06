g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s50e5k.in: 
init_time: 0.004272 
proc_time: 10.265536

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s150e10k.in: 
init_time: 0.003424 
proc_time: 0.753423

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s200e50k.in: 
init_time: 0.042411 
proc_time: 1.678621

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s500e300k.in: 
init_time: 0.172170 
proc_time: 13.170502

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s5e50.in: 
init_time: 0.000036 
proc_time: 0.000260

Running life3d-omp with (2 threads) [Succeded]
OMP tests/s20e400.in: 
init_time: 0.000145 
proc_time: 0.582386

