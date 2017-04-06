g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s50e5k.in: 
init_time: 0.004155 
proc_time: 19.877810

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s150e10k.in: 
init_time: 0.004833 
proc_time: 0.955677

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s200e50k.in: 
init_time: 0.034500 
proc_time: 2.898169

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s500e300k.in: 
init_time: 0.189664 
proc_time: 23.573658

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s5e50.in: 
init_time: 0.000110 
proc_time: 0.000272

Running life3d-omp with (1 threads) [Succeded]
OMP tests/s20e400.in: 
init_time: 0.000337 
proc_time: 1.104943

