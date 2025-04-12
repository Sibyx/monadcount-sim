//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_POINTGEOMETRY_HPP
#define MONADCOUNT_SIM_POINTGEOMETRY_HPP

#include "Geometry.hpp"

namespace monadcount_sim::models {
    class Point {
    public:
        double x, y;

    public:
        explicit Point(double x, double y) : x(x), y(y) {}
    };

    class PointGeometry : public Geometry {
    public:
        Point point;

        PointGeometry(double x, double y) : point(x, y) {}

        std::string getType() const override { return "Point"; }
    };
}

#endif //MONADCOUNT_SIM_POINTGEOMETRY_HPP
