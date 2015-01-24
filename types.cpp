#include "types.h"
#include <iostream>
namespace n_bodies {
std::ostream& operator << (std::ostream& out, const Body& body) {
    out << "mass: " << body.mass << ", ";
    out << "position ";
    for (int i = 0; i < body.position.size(); i++) {
        out << char(120 + i) << body.position[i];
    }
    out << ", " << "acceleration ";
    for (int i = 0; i < body.position.size(); i++) {
        out << char(120 + i) << body.acceleration[i];
    }
    return out;
}

} //  namespace n_bodies
