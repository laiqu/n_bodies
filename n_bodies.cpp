#include "n_bodies.h"
#include <cassert>

namespace n_bodies {
CUdevice cuDevice;
CUcontext cuContext;
CUmodule cuModule;

std::vector<Body> simulate(const std::vector<Body>& bodies, K tick,
        int dims) {
    return simulate(&bodies[0], bodies.size(), tick, dims);
}

std::vector<Body> simulate(Body const* bodies, int n, K tick, int dims) {
    if (n == 0) {
        return std::vector<Body>();
    }
    CUdeviceptr cu_bodies;
    int data_size = n * bodies[0].body_byte_size();
    cuMemAlloc(&cu_bodies, data_size);
    std::vector<K> data;
    for (int i = 0; i < n; ++i) {
        data.push_back(bodies[i].mass);
        data.insert(
            data.end(), bodies[i].position.begin(), bodies[i].position.end());
    }
    cuMemcpyHtoD(cu_bodies, &bodies[0], data_size);
    return simulate(cu_bodies, n, tick, dims);
}

std::vector<Body> simulate(
        CUdeviceptr& bodies, int n, K tick, int dims) {
    return std::vector<Body>();
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
    return true;
}
} // namespace n_bodies
