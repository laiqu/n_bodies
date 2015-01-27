#pragma once
// cannot live in config.cpp because nvcc won't find it while compiling kernel. 
#define EPS2 0.000001f
// not in namespace so it's accessible from cuda
typedef float K;
extern const int DIMENSIONS;
extern const int BRUTE_THREADS_PER_BLOCK;
