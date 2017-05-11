#!/bin/bash

# Expand all environment shorthands

# compiler
if [ "$compiler" = "gcc" ]; then
	export CXX=g++-5
	export CC=gcc-5
elif [ "$compiler" = "clang" ]; then
	export CXX=clang++-3.8
	export CC=clang-3.8
else
	echo "unrecognized compiler option: $compiler"
	exit 2
fi

# unipar
if [ "$unipar" = "TBB" ]; then
	export STRIDE_UNIPAR=TBB
elif [ "$unipar" = "OpenMP" ]; then
	export STRIDE_UNIPAR=OPENMP
	export OMP_NUM_THREADS=4
elif [ "$unipar" = "dummy" ]; then
	export STRIDE_UNIPAR=DUMMY
else
	echo "unrecognized unipar option: $unipar"
	exit 2
fi

# test
if [ "$test" = "unit" ]; then
	export GTEST_FILTER="*UnitTests__*"
elif [ "$test" = "scenarios" ]; then
	export GTEST_FILTER="*Scenarios__*"
else
	echo "Running all tests"
	export GTEST_FILTER="*"
fi
