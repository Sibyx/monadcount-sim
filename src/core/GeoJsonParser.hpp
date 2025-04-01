//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_GEOJSONPARSER_HPP
#define MONADCOUNT_SIM_GEOJSONPARSER_HPP


#include <string>
#include <vector>
#include <memory>
#include "Feature.hpp"

class GeoJSONParser {
public:
    // Parse a GeoJSON file and return a vector of Feature pointers.
    std::vector<std::unique_ptr<Feature>> parseFile(const std::string &filePath);
};

#endif //MONADCOUNT_SIM_GEOJSONPARSER_HPP
