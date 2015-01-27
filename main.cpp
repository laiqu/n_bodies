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
        bodies.push_back(n_bodies::Body::read_from_stream(std::cin, dims));
    }
    K sim_length, tick;
    std::cout << "Simulation length (in seconds):" << std::endl;
    std::cin >> sim_length;
    std::cout << "Calculate forces every (in seconds):" << std::endl;
    std::cin >> tick;
    std::cout << "Starting with: " << std::endl;
    CUdeviceptr cu_bodies = moveBodiesToDevice(bodies);
    // second one only prints raw arrays so after doing 
    // '1d;2d;3d;4d;5d;6s/.*/3 200/;$ d' on output 
    // this output easy to parse
#if 0 
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
            for (const auto& var : body.to_vector()) {
                std::cout << std::fixed << var << " ";
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
