#include "config.h"

#include <cstdio>
#include <cmath>
#define BODY_OFFSET (BODY_SINGLE_VARS + dims * BODY_REPEATED_VARS)
#define BODY(ARR, X) (ARR + (X * BODY_OFFSET))
#define MASS(BODY) (*(BODY + 0))
#define RADI(BODY) (*(BODY + 1))
#define POS(BODY) (BODY + BODY_SINGLE_VARS)
#define ACC(BODY) (BODY + dims + BODY_SINGLE_VARS)
#define VEL(BODY) (BODY + 2 * dims + BODY_SINGLE_VARS)
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
    if (x >= n) return;
    for (int i = 0; i < n; i++) {
        body_interaction(BODY(bodies, x), BODY(bodies, i), dims);
    }
}

__global__
void update_velocity(K* bodies, int n, K tick, int dims) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    if (x >= n) return;
    for (int i = 0; i < dims; ++i) {
        VEL(BODY(bodies, x))[i] += ACC(BODY(bodies, x))[i] * tick;
    }
}

__global__
void advance_bodies(K* bodies, int n, K tick, int dims) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    if (x >= n) return;
    for (int i = 0; i < dims; ++i) {
        POS(BODY(bodies, x))[i] += VEL(BODY(bodies, x))[i] * tick;
    }
}

__global__
void set_zero_to_acceleration(K* bodies, int n, int dims) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    if (x >= n) return;
    for (int i = 0; i < dims; ++i) {
        ACC(BODY(bodies, x))[i] = 0;
    }
}

__device__
bool is_nearby(K* self, K* other, int dims) {
    K dist = 0;
    for (int i = 0; i < dims; ++i) {
        K axis = POS(other)[i] - POS(self)[i];
        dist += axis * axis;
    }
    K sq_r = RADI(other) + RADI(self);
    sq_r *= sq_r;
    return dist < sq_r;
}

__device__
void merge(K* bodies, int i, int j, int dims) {
   for (int k=0; k<dims; ++k) {
       POS(BODY(bodies, i))[k] = (POS(BODY(bodies, i))[k]*MASS(BODY(bodies, i)) + POS(BODY(bodies, j))[k]*MASS(BODY(bodies, j))) / (MASS(BODY(bodies, i)) + MASS(BODY(bodies, j)));
       VEL(BODY(bodies, i))[k] = (VEL(BODY(bodies, i))[k]*MASS(BODY(bodies, i)) + VEL(BODY(bodies, j))[k]*MASS(BODY(bodies, j))) / (MASS(BODY(bodies, i)) + MASS(BODY(bodies, j)));
   }
   MASS(BODY(bodies, i)) += MASS(BODY(bodies, j));
   MASS(BODY(bodies, j)) = 0;
   RADI(BODY(bodies, i)) = powf(powf(RADI(BODY(bodies, i)), dims) + powf(RADI(BODY(bodies, j)), dims), 1.0/dims);
   RADI(BODY(bodies, j)) = 0;
}

__device__
void glue_two(K* bodies, int i, int j, int dims) {
    if (is_nearby(BODY(bodies, i), BODY(bodies, j), dims) &&
            MASS(BODY(bodies, i)) >= MASS(BODY(bodies, j))) {
        merge(bodies, i, j, dims);
    }
}

__global__
void find_merge_candidates(K* bodies, int n, int dims, int *wannabe) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    if (x >= n)
        return;
    wannabe[x] = x;
    for (int i=0; i<x; ++i) {
        if (is_nearby(BODY(bodies, i), BODY(bodies, x), dims)) {
            wannabe[x] = i;
            break;
        }
    }
}

__global__
void merge_using_candidates(K* bodies, int n, int dims, int *wannabe) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    if (x >= n)
        return;
    if (wannabe[x] != x)
        return;

    for (int i=x+1; i<n; ++i) {
        if (wannabe[i] == x) {
            merge(bodies, x, i, dims);
        }
    }
}

__global__
void glue_nearby(K* bodies, int n, int dims) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            glue_two(bodies, i, j, dims);
        }
    }
}

__global__
void glue_nearby_parallel(K* bodies, int n, int dims, int d, int BLOCKS_Y) {
    int x = 1024 * (BLOCKS_Y * blockIdx.x + blockIdx.y) + 32 * threadIdx.x + threadIdx.y;

    if (x % (2 * d) >= d || x + d >= n) return;

    int i = x;
    int j = x + d;
    glue_two(bodies, i, j, dims);
    __syncthreads();

    i = x + d;
    j = i + d;
    if (j >= n) return;
    glue_two(bodies, i, j, dims);
}
}
