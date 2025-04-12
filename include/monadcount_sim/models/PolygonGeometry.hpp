//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_POLYGONGEOMETRY_HPP
#define MONADCOUNT_SIM_POLYGONGEOMETRY_HPP

#include "Geometry.hpp"
#include "PointGeometry.hpp"
#include <vector>

namespace monadcount_sim::models {
    class PolygonGeometry : public Geometry {
    public:
        std::vector<std::vector<Point>> rings;
        std::string getType() const override { return "Polygon"; }
    };
}

#endif //MONADCOUNT_SIM_POLYGONGEOMETRY_HPP
