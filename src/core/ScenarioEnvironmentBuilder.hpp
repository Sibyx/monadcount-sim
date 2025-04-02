//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
#define MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP


#include <vector>
#include <memory>
#include "Feature.hpp" // Adjust the path as needed.
#include "ScenarioEnvironment.hpp"

class ScenarioEnvironmentBuilder {
public:
    // Build the environment from the parsed Feature objects.
    std::unique_ptr<ScenarioEnvironment> Build(const std::vector<std::unique_ptr<Feature>> &features);

private:
    // Helper methods for each feature type.
    void createApNode(const Feature &feature, ScenarioEnvironment &env);
    void createSnifferNode(const Feature &feature, ScenarioEnvironment &env);
    void createTerminalNode(const Feature &feature, ScenarioEnvironment &env);
    void createObstacle(const Feature &feature, ScenarioEnvironment &env);
    void createSeat(const Feature &feature, ScenarioEnvironment &env);
    void createDoor(const Feature &feature, ScenarioEnvironment &env);
};


#endif //MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
