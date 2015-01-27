#pragma once
#include "config.h"

#include <algorithm>
#include <iostream>
#include <initializer_list>

// outside of namespace because of CUDA
namespace n_bodies {

// Describes bodies.
//
// Note that this struct isn't passed to CUDA kernel, but instead array of
// all variables is passed.
// While adding variables to struct, you have to bump consts (SINGLE_VARS and 
// REPEATED_VARS) and add your variable to to_vector method.
// TODO(laiqu) maybe figure out more convenient way to pass variables to kernel
// because currently all variables are constrained to be type of K
struct Body {
    K mass;
    K radius;
    std::vector<K> position;
    std::vector<K> acceleration;
    std::vector<K> velocity;
    Body() {}
    Body(K mass, K radius,
         std::initializer_list<K> position,
         std::initializer_list<K> acceleration,
         std::initializer_list<K> velocity) :
        mass(mass), radius(radius),
        position(position), acceleration(acceleration), velocity(velocity) {}
    Body(K* variables, int dims) {
        mass = variables[0];
        radius = variables[1];
        position = std::vector<K>(variables + BODY_SINGLE_VARS,
                                  variables + dims + BODY_SINGLE_VARS);
        acceleration = std::vector<K>(variables + dims + BODY_SINGLE_VARS,
                                      variables + 2 * dims + BODY_SINGLE_VARS);
        velocity = std::vector<K>(variables + 2 * dims + BODY_SINGLE_VARS,
                                  variables + 3 * dims + BODY_SINGLE_VARS);
    }
    int body_byte_size() const {
        return (BODY_SINGLE_VARS +
                position.size() + acceleration.size() + velocity.size())
            * sizeof(K);
    }
    std::vector<K> to_vector() const {
        std::vector<K> vec;
        vec.push_back(mass);
        vec.push_back(radius);
        vec.insert(vec.end(), position.begin(), position.end());
        vec.insert(vec.end(), acceleration.begin(), acceleration.end());
        vec.insert(vec.end(), velocity.begin(), velocity.end());
        return vec;
    }

    static Body read_from_stream(std::istream& stream, int dims) {
        K input[variable_count(dims)];
        for (int j = 0; j < variable_count(dims); j++) {
            stream >> input[j];
        }
        return Body(input, dims);
    }

    static int body_byte_size(int dims) {
        return (BODY_SINGLE_VARS + dims * BODY_REPEATED_VARS) * sizeof(K);
    }

    static int variable_count(int dims) {
        return BODY_SINGLE_VARS + BODY_REPEATED_VARS * dims;
    }
};

std::ostream& operator << (std::ostream& out, const Body& body);

} //  namespace n_bodies
