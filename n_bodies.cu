#include "config.h"
#include "types.h"

#include <cstdio>
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

__global__
void glue_nearby(K* bodies, int n, int dims) {
    // TODO(laiqu) implement this as proper Kernel.
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            if (is_nearby(BODY(bodies, i), BODY(bodies, j), dims) &&
                    MASS(BODY(bodies, i)) >= MASS(BODY(bodies, j))) {
               MASS(BODY(bodies, i)) += MASS(BODY(bodies, j));
               MASS(BODY(bodies, j)) = 0;
               RADI(BODY(bodies, i)) += RADI(BODY(bodies, j));
               RADI(BODY(bodies, j)) = 0;
            }
        }
    }
}
}
