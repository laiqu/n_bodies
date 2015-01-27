#include "n_bodies.h"
#include "types.h"
#include "config.h"
#include <vector>
#include <iostream>

int main(int argc, char* argv[]) {
    int n, dims;
    bool interactive = false;
    if (!n_bodies::init()) {
        std::cout << "Failed to initialize n_bodies" << std::endl;
        exit(1);
    }
    // Since currently we have only one flag...
    // If there is a flag, then the output is pretty and verbose,
    // otherwise the output is in a way current visualization expects it to be.
    if (argc > 1) {
       interactive = true; 
    }
    std::vector<n_bodies::Body> bodies;
    if (interactive)
        std::cout << "Number of dimensions?:" << std::endl;
    std::cin >> dims;
    if (interactive)
        std::cout << "Number of bodies?:" << std::endl;
    std::cin >> n;
    if (interactive)
        std::cout << "Describe bodies (mass, radius, pos_x, pos_y, pos_z, acc_x, acc_y, acc_z, vel_x, vel_y, vel_z, assuming 3 dimensions):" << std::endl;
    for (int i = 0; i < n; i++) {
        bodies.push_back(n_bodies::Body::read_from_stream(std::cin, dims));
    }
    K sim_length, tick;
    if (interactive)
        std::cout << "Simulation length (in seconds):" << std::endl;
    std::cin >> sim_length;
    if (interactive)
        std::cout << "Calculate forces every (in seconds):" << std::endl;
    std::cin >> tick;
    CUdeviceptr cu_bodies = moveBodiesToDevice(bodies);
    // second one only prints raw arrays so after doing 
    // '1d;2d;3d;4d;5d;6s/.*/3 200/;$ d' on output 
    // this output easy to parse
    if (interactive) {
        std::cout << "Starting with: " << std::endl;
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
    } else {
        std::cout << dims << " " << n << std::endl;
        for (K i = 0; i < sim_length; i += tick) {
            auto result = n_bodies::simulate(cu_bodies, n, tick, dims);
            std::cout << i << std::endl;
            for (const auto& body : result) {
                for (const auto& var : body.to_vector()) {
                    std::cout << std::fixed << var << " ";
                }
                std::cout << std::endl;
            }
        }
        std::cout << sim_length << " " << tick << std::endl;
    }
    if (interactive)
        std::cout << "Simulation duration: " << sim_length << " with forces "
            "recalculation every: " << tick << std::endl;
    cuMemFree(cu_bodies);
    return 0;
}
