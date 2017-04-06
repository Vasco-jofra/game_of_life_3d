g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d.cpp -o life3d
g++ -std=c++11 -fopenmp -lgomp -g -Wall -Wextra life3d-omp.cpp -o life3d-omp

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s50e5k.in: 
init_time: 0.001753 
proc_time: 3.593496

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s150e10k.in: 
init_time: 0.003451 
proc_time: 0.328082

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s200e50k.in: 
init_time: 0.017978 
proc_time: 0.868402

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s500e300k.in: 
init_time: 0.117908 
proc_time: 5.632293

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s5e50.in: 
init_time: 0.000036 
proc_time: 0.000368

Running life3d-omp with (8 threads) [Succeded]
OMP tests/s20e400.in: 
init_time: 0.000185 
proc_time: 0.245699

