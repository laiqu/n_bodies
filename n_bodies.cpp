#include "n_bodies.h"
#include "config.h"
#include <cassert>

namespace n_bodies {
CUdevice cuDevice;
CUcontext cuContext;
CUmodule cuModule;
CUfunction cuBruteCalculateInteractions;
CUfunction cuUpdateVelocity;
CUfunction cuAdvanceBodies;
CUfunction cuSetZeroToAcceleration;
CUfunction cuGlueNearby;
CUfunction cuGlueNearbyParallel;
CUfunction cuFindMergeCandidates;
CUfunction cuMergeUsingCandidates;

CUdeviceptr candidates = 0;

CUdeviceptr moveBodiesToDevice(const std::vector<Body>& bodies) {
    return moveBodiesToDevice(&bodies[0], bodies.size());
}

CUdeviceptr moveBodiesToDevice(Body const* bodies, int n) {
    CUdeviceptr cu_bodies;
    int data_size = n * bodies[0].body_byte_size();
    cuMemAlloc(&cu_bodies, data_size);

    std::vector<K> data;
    for (int i = 0; i < n; ++i) {
        std::vector<K> body_vec = bodies[i].to_vector();
        data.insert(data.end(), body_vec.begin(), body_vec.end());
    }
    cuMemcpyHtoD(cu_bodies, &data[0], data_size);
    return cu_bodies;
}

std::vector<Body> simulate(CUdeviceptr& bodies, int& n, K tick, int dims) {
    int blocks = (n + BRUTE_THREADS_PER_BLOCK - 1) / BRUTE_THREADS_PER_BLOCK;
    void* calculate_args[] = {&bodies, &n, &dims}; 
    CUresult res; 
    res = cuLaunchKernel(cuSetZeroToAcceleration, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, calculate_args, 0);
    res = cuLaunchKernel(cuBruteCalculateInteractions, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, calculate_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }
    void* advance_args[] = {&bodies, &n, &tick, &dims};
    res = cuLaunchKernel(cuUpdateVelocity, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, advance_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }
    res = cuLaunchKernel(cuAdvanceBodies, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, advance_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }

    if (!candidates) {
        res = cuMemAlloc(&candidates, n*sizeof(int));
        if (res != CUDA_SUCCESS) {
            throw 0;
        }
    }

    void *merge_args[] = {&bodies, &n, &dims, &candidates};

    res = cuLaunchKernel(cuFindMergeCandidates, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, merge_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }
    res = cuLaunchKernel(cuMergeUsingCandidates, blocks, 1, 1,
                         BRUTE_THREADS_PER_BLOCK, 1, 1,
                         0, 0, merge_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }

    // void* glue_args[] = {&bodies, &n, &dims};
    // res = cuLaunchKernel(cuGlueNearby, 1, 1, 1,
    //                      1, 1, 1,
    //                      0, 0, glue_args, 0);
    // if (res != CUDA_SUCCESS) {
    //     throw 0;
    // }

    // TODO: what if n > 1024 * 256 * 256
    /*
    int temp = n / 1024 + 1;
    int BLOCKS_X = 1, BLOCKS_Y = 1;
    if (temp > 1) {
        if (temp > 256) {
            BLOCKS_X = 256;
            BLOCKS_Y = temp / 256 + 1;
        } else {
            BLOCKS_X = temp;
            BLOCKS_Y = 1;
        }
    }
    void* glue_parallel_args[] = {&bodies, &n, &dims, NULL, &BLOCKS_Y};
    for (int i=1; i<n; i++) {
        glue_parallel_args[3] = &i;
        res = cuLaunchKernel(cuGlueNearbyParallel,
                          BLOCKS_X, BLOCKS_Y, 1,
                          THREADS_X, THREADS_Y, 1,
                          0, 0, glue_parallel_args, 0);

        if (res != CUDA_SUCCESS) {
            throw 0;
        }
    }
    */
    K data[n * Body::body_byte_size(dims)];
    cuMemcpyDtoH(data, bodies, n * Body::body_byte_size(dims));
    std::vector<Body> result;
    for (int i = 0, j = 0; i < n; i++, j += Body::variable_count(dims)) {
        Body body(data + j, dims);
        if (body.mass <= EPS2) {
            for (int k = 0; k < Body::variable_count(dims); k++) {
                std::swap(*(data + j + k),
                          *(data + (n - 1) * Body::variable_count(dims) + k));
            }
            i--;
            j -= Body::variable_count(dims);
            n--;
        } else {   
            result.push_back(body); 
        }
    }
    cuMemcpyHtoD(bodies, data, n * Body::body_byte_size(dims));
    return result;
}

bool init() {
    // TODO(laiqu) add a little more descriptive return value.
    cuInit(0);
    CUresult res = cuDeviceGet(&cuDevice, 0);
    if (res != CUDA_SUCCESS){
        return false;
    }

    res = cuCtxCreate(&cuContext, 0, cuDevice);
    if (res != CUDA_SUCCESS){
        return false;
    }    

    res = cuModuleLoad(&cuModule, "n_bodies.ptx");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuBruteCalculateInteractions,
                              cuModule, "brute_calculate_interactions");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuAdvanceBodies,
                              cuModule, "advance_bodies");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuUpdateVelocity,
                              cuModule, "update_velocity");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuSetZeroToAcceleration,
                              cuModule, "set_zero_to_acceleration");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuGlueNearby,
                              cuModule, "glue_nearby");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuGlueNearbyParallel,
                              cuModule, "glue_nearby_parallel");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuFindMergeCandidates,
                              cuModule, "find_merge_candidates");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    res = cuModuleGetFunction(&cuMergeUsingCandidates,
                              cuModule, "merge_using_candidates");
    if (res != CUDA_SUCCESS) {
        return false;
    }

    return true;
}
} // namespace n_bodies
