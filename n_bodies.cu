#include "config.h"

#include <cstdio>
#define BODY_OFFSET (1 + dims * 2)
#define BODY(ARR, X) (ARR + (X * BODY_OFFSET))
#define MASS(BODY) (*(BODY + 0))
#define POS(BODY) (BODY + 1)
#define ACC(BODY) (BODY + dims + 1)
// macro usage - MASS(BODY(bodies, i)) or POS(BODY(bodies, i))[1] (this gives y)
extern "C" {
__device__
void body_interaction(K* self, K* other, int dims) {
    K dist = EPS2;
    for (int i = 0; i < dims; ++i) {
        K axis = POS(other)[i] - POS(self)[i];
        dist += axis * axis;
    }
    dist = dist * dist * dist;
    dist = K(1) / sqrtf(dist);
    dist = MASS(other) * dist;
    for (int i = 0; i < dims; ++i) {
        ACC(self)[i] += (POS(other)[i] - POS(self)[i]) * dist;
    }
}

__global__
void brute_calculate_interactions(K* bodies, int n, int dims) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= n || y >= n) return;
    body_interaction(BODY(bodies, x), BODY(bodies, y), dims);
}

__global__
void advance_bodies(K* bodies, int n, K tick, int dims) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= n || y >= n) return;
    for (int i = 0; i < dims; ++i) {
        POS(BODY(bodies, x))[i] += ACC(BODY(bodies, y))[i] * tick;
    }
}
}
