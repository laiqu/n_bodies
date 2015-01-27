#include "n_bodies.h"
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

std::vector<Body> simulate(CUdeviceptr& bodies, int n, K tick, int dims) {
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

    void* glue_args[] = {&bodies, &n, &dims};
    res = cuLaunchKernel(cuGlueNearby, 1, 1, 1,
                         1, 1, 1,
                        0, 0, glue_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }
    
    K data[n * Body::body_byte_size(dims)];
    cuMemcpyDtoH(data, bodies, n * Body::body_byte_size(dims));
    std::vector<Body> result(n);
    for (int i = 0, j = 0; i < n; i++, j += Body::variable_count(dims)) {
        result[i] = Body(data + j, dims);
    }
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
                              
    return true;
}
} // namespace n_bodies
