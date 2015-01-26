#pragma once
// not in namespace so it's accessible from cuda
typedef float K;
const int DIMENSIONS = 3;
const K EPS2 = 0.0000000000001;
const int BRUTE_THREADS_PER_BLOCK = 32;
