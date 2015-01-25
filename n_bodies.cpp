#include "n_bodies.h"
#include <cassert>

namespace n_bodies {
CUdevice cuDevice;
CUcontext cuContext;
CUmodule cuModule;
CUfunction cuBruteCalculateInteractions;
CUfunction cuAdvanceBodies;

CUdeviceptr moveBodiesToDevice(const std::vector<Body>& bodies) {
    return moveBodiesToDevice(&bodies[0], bodies.size());
}

CUdeviceptr moveBodiesToDevice(Body const* bodies, int n) {
    CUdeviceptr cu_bodies;
    int data_size = n * bodies[0].body_byte_size();
    cuMemAlloc(&cu_bodies, data_size);
    std::vector<K> data;
    for (int i = 0; i < n; ++i) {
        data.push_back(bodies[i].mass);
        data.insert(
            data.end(), bodies[i].position.begin(), bodies[i].position.end());  
        data.insert(
            data.end(),
            bodies[i].acceleration.begin(), bodies[i].acceleration.end());  
    }
    cuMemcpyHtoD(cu_bodies, &data[0], data_size);
    return cu_bodies;
}

std::vector<Body> simulate(CUdeviceptr& bodies, int n, K tick, int dims) {
    int blocks = (n + BRUTE_THREADS_PER_BLOCK - 1) / BRUTE_THREADS_PER_BLOCK;
    void* calculate_args[] = {&bodies, &n, &dims}; 
    CUresult res; 
    res = cuLaunchKernel(cuBruteCalculateInteractions, blocks, blocks, 1,
                         BRUTE_THREADS_PER_BLOCK, BRUTE_THREADS_PER_BLOCK, 1,
                         0, 0, calculate_args, 0);
    if (res != CUDA_SUCCESS) {
        throw 0;
    }
    void* advance_args[] = {&bodies, &n, &tick, &dims};
    res = cuLaunchKernel(cuAdvanceBodies, blocks, blocks, 1,
                         BRUTE_THREADS_PER_BLOCK, BRUTE_THREADS_PER_BLOCK, 1,
                         0, 0, advance_args, 0);
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
                              
    return true;
}
} // namespace n_bodies
