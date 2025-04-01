//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_FEATURE_HPP
#define MONADCOUNT_SIM_FEATURE_HPP

#include <string>
#include <memory>
#include "Geometry.hpp"
#include "Category.hpp"

class Feature {
private:
    std::string id;
    std::string experiment_id;
    Category category;
    std::unique_ptr<Geometry> geometry;

public:
    Feature(const std::string &id,
            const std::string &experiment_id,
            const Category &category,
            std::unique_ptr<Geometry> geometry)
            : id(id), experiment_id(experiment_id),
              category(category), geometry(std::move(geometry)) {}

    [[nodiscard]] const std::string &getId() const { return id; }

    [[nodiscard]] const std::string &getExperimentId() const { return experiment_id; }

    [[nodiscard]] const Category &getCategory() const { return category; }

    [[nodiscard]] const Geometry *getGeometry() const { return geometry.get(); }
};

#endif //MONADCOUNT_SIM_FEATURE_HPP
