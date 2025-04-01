//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
#define MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP


#include <vector>
#include <memory>
#include "Feature.hpp" // Adjust the path as needed.
#include "ScenarioEnvironment.hpp"

class Ns3EnvironmentBuilder {
public:
    // Build the environment from the parsed Feature objects.
    std::unique_ptr<Ns3Environment> Build(const std::vector<std::unique_ptr<Feature>> &features);

private:
    // Helper methods for each feature type.
    void createApNode(const Feature &feature, Ns3Environment &env);
    void createSnifferNode(const Feature &feature, Ns3Environment &env);
    void createTerminalNode(const Feature &feature, Ns3Environment &env);
    void createObstacle(const Feature &feature, Ns3Environment &env);
    void createSeat(const Feature &feature, Ns3Environment &env);
    void createDoor(const Feature &feature, Ns3Environment &env);
};


#endif //MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
