#pragma once
// not in namespace so it's accessible from cuda
typedef float K;
const int DIMENSIONS = 2;
const K EPS2 = 0.1f;
const int BRUTE_THREADS_PER_BLOCK = 32;
