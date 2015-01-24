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
// method.
// TODO(laiqu) maybe figure out more convenient way to pass variables to kernel
// because currently all variables are constrained to be type of K
struct Body {
    K mass;
    std::vector<K> position;
    std::vector<K> acceleration;
    Body() {}
    Body(K mass, std::initializer_list<K> position,
            std::initializer_list<K> acceleration) :
        mass(mass), position(position), acceleration(acceleration) {}
    int body_byte_size() const {
        return (1 + position.size() + acceleration.size()) * sizeof(K);
    }
};

std::ostream& operator << (std::ostream& out, const Body& body);

} //  namespace n_bodies
