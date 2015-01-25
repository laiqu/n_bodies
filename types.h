#pragma once
#include "config.h"

#include <algorithm>
#include <initializer_list>

namespace n_bodies {

// Describes bodies.
//
// Note that this struct isn't passed to CUDA kernel, but instead array of
// positions and mass is passed.
// While adding variables to struct, you probably want to update body_byte_size
// method, access macros in kernel and moveBodiesToDevice function.
// TODO(laiqu) maybe figure out more convenient way to pass variables to kernel
// because currently all variables are constrained to be type of K
struct Body {
    K mass;
    std::vector<K> position;
    std::vector<K> acceleration;
    std::vector<K> velocity;
    Body() {}
    Body(K mass, std::initializer_list<K> position,
         std::initializer_list<K> acceleration,
         std::initializer_list<K> velocity) :
        mass(mass), position(position), acceleration(acceleration),
        velocity(velocity) {}
    Body(K* variables, int dims) {
        mass = variables[0];
        position = std::vector<K>(variables + 1,
                                  variables + dims + 1);
        acceleration = std::vector<K>(variables + dims + 1,
                                      variables + 2 * dims + 1);
        velocity = std::vector<K>(variables + 2 * dims + 1,
                                  variables + 3 * dims + 1);
    }
    int body_byte_size() const {
        return (1 + position.size() + acceleration.size() + velocity.size())
            * sizeof(K);
    }

    static int body_byte_size(int dims) {
        return (1 + dims * 3) * sizeof(K);
    }

    static int variable_count(int dims) {
        return 1 + 3 * dims;
    }
};

std::ostream& operator << (std::ostream& out, const Body& body);

} //  namespace n_bodies
