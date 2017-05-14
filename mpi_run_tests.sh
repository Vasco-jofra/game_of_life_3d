if ! make mpi ; then
    exit 1;
fi


tests_dir="tests"
their_out_dir="tests/out"
our_out_dir="tests/our_out"

declare -A tests
tests[s5e50]=10
tests[s20e400]=500
tests[s150e10k]=1000
tests[s200e50k]=1000
tests[s50e5k]=300
tests[s500e300k]=2000

 #tests[s500e5M]=10
# tests[s600e20M]=5
# tests[custom_s1000e650000]=750
# tests[custom_s2500e50000]=100
# tests[custom_s10000e7500000]=10
# tests[custom_s5000e10000000]=50

file_name="life3d-mpi"
num_processes=4

if [ ! -z "$1" ]; then
	num_processes=$1
fi

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
		echo -en "\e[1;38;5;215mRunning \033[32m$f \e[1;38;5;215m$in \e[0m > $our_out ($num_processes processes)"
		# echo -en "Running $f with ($OMP_NUM_THREADS threads) "
		if ! mpirun -np $num_processes $f $in $gen > $our_out ; then
			exit 1;
		fi
		output=$(diff -q $our_out $out)
		if [[ $output ]]; then
			echo -e "  [ \e[1;38;5;196mFailed\e[0m ]"
			echo "Test								Expected"
			diff -y $out $our_out
		else
			echo -e "  [ \e[1;38;5;41mSucceded\e[0m ]"
			# echo "[Succeded]"
		fi
		cat time.log
	done
	echo
done
