#include "types.h"
#include <iostream>
namespace n_bodies {
std::ostream& operator << (std::ostream& out, const Body& body) {
    out << "mass: " << body.mass << ", ";
    out << "position ";
    for (int i = 0; i < body.position.size(); i++) {
        out << char('x' + i) << ": " << body.position[i] << " "; 
    }
    out << ", " << "acceleration ";
    for (int i = 0; i < body.acceleration.size(); i++) {
        out << char('x' + i) << ": " << body.acceleration[i] << " ";
    }
    out << ", " << "velocity ";
    for (int i = 0; i < body.velocity.size(); i++) {
        out << char('x' + i) << ": " << body.velocity[i] << " ";
    }
    return out;
}

} //  namespace n_bodies
