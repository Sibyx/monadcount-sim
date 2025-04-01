//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_POINTGEOMETRY_HPP
#define MONADCOUNT_SIM_POINTGEOMETRY_HPP

#include "Geometry.hpp"

struct Point {
    double x, y;
};

class PointGeometry : public Geometry {
public:
    Point point;
    PointGeometry(double x, double y) : point({x, y}) {}
    std::string getType() const override { return "Point"; }
};

#endif //MONADCOUNT_SIM_POINTGEOMETRY_HPP
