g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s50e5k.in: 
init_time: 0.001634 
proc_time: 5.545671

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s150e10k.in: 
init_time: 0.003441 
proc_time: 0.425246

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s200e50k.in: 
init_time: 0.046307 
proc_time: 0.890244

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s500e300k.in: 
init_time: 0.139004 
proc_time: 6.820457

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s5e50.in: 
init_time: 0.000049 
proc_time: 0.000428

Running life3d-omp with (4 threads) [Succeded]
OMP tests/s20e400.in: 
init_time: 0.000174 
proc_time: 0.318339

