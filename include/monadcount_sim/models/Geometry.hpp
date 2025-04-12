//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_GEOMETRY_HPP
#define MONADCOUNT_SIM_GEOMETRY_HPP

#include <string>

namespace monadcount_sim::models {
    class Geometry {
    public:
        virtual ~Geometry() = default;
        virtual std::string getType() const = 0;
    };
}

#endif //MONADCOUNT_SIM_GEOMETRY_HPP
