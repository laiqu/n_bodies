#pragma once
// cannot live in config.cpp because nvcc won't find it while compiling kernel. 
#define EPS2 0.000001f
// not in namespace so it's accessible from cuda
typedef float K;
const int DIMENSIONS = 3;
const int BRUTE_THREADS_PER_BLOCK = 32;
const int BODY_SINGLE_VARS = 2;
const int BODY_REPEATED_VARS = 3;
const int THREADS_X = 32;
const int THREADS_Y = 32;
int BLOCKS_X = 256;
int BLOCKS_Y = 256;