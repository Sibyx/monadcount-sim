#include <fstream>
#include <stdexcept>
#include <iostream>

#include <nlohmann/json.hpp>
#include <monadcount_sim/models/Feature.hpp>
#include <monadcount_sim/core/GeoJsonParser.hpp>
#include "monadcount_sim/models/PointGeometry.hpp"
#include "monadcount_sim/models/PolygonGeometry.hpp"


std::vector<std::unique_ptr<monadcount_sim::models::Feature>> monadcount_sim::core::GeoJSONParser::parseFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    nlohmann::json j;
    file >> j;

    if (j["type"] != "FeatureCollection") {
        throw std::runtime_error("Invalid GeoJSON: Not a FeatureCollection");
    }

    std::vector<std::unique_ptr<monadcount_sim::models::Feature>> features;
    for (const auto &featureJson : j["features"]) {
        try {
            // Extract common properties.
            std::string id = featureJson["properties"]["id"].get<std::string>();
            std::string experiment_id = featureJson["properties"]["experiment_id"].get<std::string>();
            std::string catStr = featureJson["properties"]["category"].get<std::string>();
            monadcount_sim::models::Category cat = monadcount_sim::models::Category::fromString(catStr);

            // Parse geometry.
            std::string geomType = featureJson["geometry"]["type"].get<std::string>();
            std::unique_ptr<monadcount_sim::models::Geometry> geometry;

            if (geomType == "Point") {
                const auto &coords = featureJson["geometry"]["coordinates"];
                if (!coords.is_array() || coords.size() < 2)
                    throw std::runtime_error("Invalid Point coordinates");
                double x = coords[0].get<double>();
                double y = coords[1].get<double>();
                geometry = std::make_unique<monadcount_sim::models::PointGeometry>(x, y);
            } else if (geomType == "Polygon") {
                const auto &coords = featureJson["geometry"]["coordinates"];
                auto polyGeom = std::make_unique<monadcount_sim::models::PolygonGeometry>();
                for (const auto &ring : coords) {
                    std::vector<monadcount_sim::models::Point> loop;
                    for (const auto &pt : ring) {
                        if (!pt.is_array() || pt.size() < 2)
                            throw std::runtime_error("Invalid polygon point");
                        double x = pt[0].get<double>();
                        double y = pt[1].get<double>();
                        loop.push_back(monadcount_sim::models::Point(x, y));
                    }
                    polyGeom->rings.push_back(loop);
                }
                geometry = std::move(polyGeom);
            } else {
                throw std::runtime_error("Unsupported geometry type: " + geomType);
            }

            features.push_back(std::make_unique<monadcount_sim::models::Feature>(id, experiment_id, cat, std::move(geometry)));
        } catch (const std::exception &ex) {
            std::cerr << "Error parsing feature: " << ex.what() << std::endl;
        }
    }

    return features;
}
