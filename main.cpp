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
    float tick = 0.1f;
    CUdeviceptr cu_bodies = moveBodiesToDevice(bodies);
    auto result = n_bodies::simulate(cu_bodies, n,tick);
    for (const auto& body : result) {
        std::cout << body << std::endl; 
    }
    cuMemFree(cu_bodies);
    return 0;
}
