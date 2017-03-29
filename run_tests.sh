#!/bin/bash

echo "
#########
COMPILING
#########"

if ! make ; then
    exit 1;
fi

echo

tests_dir="tests"
their_out_dir="tests/out"
our_out_dir="tests/our_out"

declare -A tests
# tests[s150e10k]=1000
tests[s5e50]=10

# file_name="life3d life3d-omp"
file_name="life3d"
# file_name="life3d-omp"

if [ ! -z "$1" ]; then
	file_name=$1
fi

for t in "${!tests[@]}"; do
	gen=${tests[$t]}
	in="tests/$t.in"
	out="tests/out/$t.$gen.out"
	our_out="tests/our_out/$t.$gen.our_out"
	if [ ! -f $in ] || [ ! -f $out ]; then
		echo "input: $in, or output: $out files do not exist."
		continue
	fi

	for f in $file_name; do
		echo -en "\e[1;38;5;215mRunning \033[32m$f \e[1;38;5;215m$in \e[0m > $our_out"
		if ! ./$f $in $gen > $our_out ; then
			exit 1;
		fi
		output=$(diff -q $our_out $out)
		if [[ $output ]]; then
			echo -e "  [ \e[1;38;5;196mFailed\e[0m ]"
			echo "Test								Expected"
			diff -C 5 $out $our_out
		else
			echo -e "  [ \e[1;38;5;41mSucceded\e[0m ]"
		fi
		cat time.log
	done
	echo
done