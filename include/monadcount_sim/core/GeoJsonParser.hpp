//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_GEOJSONPARSER_HPP
#define MONADCOUNT_SIM_GEOJSONPARSER_HPP


#include <string>
#include <vector>
#include <memory>

#include <monadcount_sim/models/Feature.hpp>

namespace monadcount_sim::core {
    class GeoJSONParser {
    public:
        // Parse a GeoJSON file and return a vector of Feature pointers.
        std::vector<std::unique_ptr<monadcount_sim::models::Feature>> parseFile(const std::string &filePath);
    };
}

#endif //MONADCOUNT_SIM_GEOJSONPARSER_HPP
