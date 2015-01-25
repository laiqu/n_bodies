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
    std::cout << "Number of bodies?:" << std::endl;
    std::cin >> n;
    std::cout << "Describe bodies (mass, pos_x, pos_y, acc_x, acc_y):" << std::endl;
    for (int i = 0; i < n; i++) {
        float mass, pos_x, pos_y, acc_x, acc_y;
        std::cin >> mass >> pos_x >> pos_y >> acc_x >> acc_y;
        bodies.push_back(n_bodies::Body(mass, {pos_x, pos_y}, {acc_x, acc_y}));
    }
    K sim_length, tick;
    std::cout << "Simulation length (in seconds):" << std::endl;
    std::cin >> sim_length;
    std::cout << "Calculate forces every (in seconds):" << std::endl;
    std::cin >> tick;
    std::cout << "Starting with: " << std::endl;
    for (const auto& body : bodies) {
        std::cout << std::fixed << body << std::endl; 
    }
    CUdeviceptr cu_bodies = moveBodiesToDevice(bodies);
    for (K i = 0; i < sim_length; i += tick) {
        auto result = n_bodies::simulate(cu_bodies, n, tick);
        std::cout << "After " << i << ":" << std::endl;
        for (const auto& body : result) {
            std::cout << std::fixed << body << std::endl;  
        }
    }
    std::cout << "Simulation duration: " << sim_length << " with forces "
        "recalculation every: " << tick << std::endl;
    cuMemFree(cu_bodies);
    return 0;
}
