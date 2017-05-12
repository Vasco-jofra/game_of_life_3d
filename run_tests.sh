#!/bin/bash

if ! make ; then
    exit 1;
fi

echo

tests_dir="tests"
their_out_dir="tests/out"
our_out_dir="tests/our_out"

declare -A tests
# tests[s5e50]=10
# tests[s20e400]=500
# tests[s150e10k]=1000
# tests[s200e50k]=1000
# tests[s50e5k]=300
tests[s500e300k]=2000
# tests[custom_s1000e650000]=750
# tests[custom_s2500e50000]=100
# tests[custom_s10000e7500000]=10
# tests[custom_s5000e10000000]=50

# file_name="life3d life3d-omp"
# file_name="life3d"
file_name="life3d-omp"
num_threads=8

if [ ! -z "$1" ]; then
	file_name=$1
fi

export OMP_NUM_THREADS=$num_threads
for t in "${!tests[@]}"; do
	gen=${tests[$t]}
	in="tests/$t.in"
	out="tests/out/$t.$gen.out"
	our_out="tests/our_out/$t.$gen.our_out"
	if [ ! -f $in ]; then
		echo "input: $in file does not exist."
		continue
	fi

	if [ ! -f $out ]; then
		echo "output: $out file does not exist."
	fi


	for f in $file_name; do
		echo -en "\e[1;38;5;215mRunning \033[32m$f \e[1;38;5;215m$in \e[0m > $our_out ($OMP_NUM_THREADS threads)"
		# echo -en "Running $f with ($OMP_NUM_THREADS threads) "
		if ! ./$f $in $gen > $our_out ; then
			exit 1;
		fi
		output=$(diff -q $our_out $out)
		if [[ $output ]]; then
			echo -e "  [ \e[1;38;5;196mFailed\e[0m ]"
			# echo "Test								Expected"
			# diff -y $out $our_out
		else
			echo -e "  [ \e[1;38;5;41mSucceded\e[0m ]"
			# echo "[Succeded]"
		fi
		cat time.log
	done
	echo
done

export OMP_NUM_THREADS=
