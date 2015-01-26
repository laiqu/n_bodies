#include "n_bodies.h"
#include "types.h"
#include <vector>
#include <iostream>

int main() {
    int n, dims;
    if (!n_bodies::init()) {
        std::cout << "Failed to initialize n_bodies" << std::endl;
        exit(1);
    }
    std::vector<n_bodies::Body> bodies;
    std::cout << "Number of dimensions?:" << std::endl;
    std::cin >> dims;
    std::cout << "Number of bodies?:" << std::endl;
    std::cin >> n;
    std::cout << "Describe bodies (mass, pos_x, pos_y, vel_x, vel_y):" << std::endl;
    for (int i = 0; i < n; i++) {
        K input[n_bodies::Body::variable_count(dims)];
        for (int j = 0; j < n_bodies::Body::variable_count(dims); ++j) {
            if (j >= dims + 1 && j < 2 * dims + 1) {
                input[j] = 0;
            } else {
                std::cin >> input[j];
            }
        }

        bodies.push_back(n_bodies::Body(input, dims));
    }
    K sim_length, tick;
    std::cout << "Simulation length (in seconds):" << std::endl;
    std::cin >> sim_length;
    std::cout << "Calculate forces every (in seconds):" << std::endl;
    std::cin >> tick;
    std::cout << "Starting with: " << std::endl;
    CUdeviceptr cu_bodies = moveBodiesToDevice(bodies);
#if 1
    for (const auto& body : bodies) {
        std::cout << std::fixed << body << std::endl; 
    }
    for (K i = 0; i < sim_length; i += tick) {
        auto result = n_bodies::simulate(cu_bodies, n, tick);
        std::cout << "After " << i << ":" << std::endl;
        for (const auto& body : result) {
            std::cout << std::fixed << body << std::endl;  
        }
    }
#else
    for (K i = 0; i < sim_length; i += tick) {
        auto result = n_bodies::simulate(cu_bodies, n, tick);
        std::cout << i << std::endl;
        for (const auto& body : result) {
            std::cout << std::fixed << body.mass << " ";
            for (int j = 0; j < dims; ++j) {
                std::cout << std::fixed << body.position[j] << " ";
            }
            std::cout << std::endl;
        }
    }
#endif
    std::cout << "Simulation duration: " << sim_length << " with forces "
        "recalculation every: " << tick << std::endl;
    cuMemFree(cu_bodies);
    return 0;
}
