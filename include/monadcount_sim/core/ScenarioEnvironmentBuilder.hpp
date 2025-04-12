//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
#define MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP


#include <vector>
#include <memory>
#include <monadcount_sim/models/Feature.hpp>

#include "ScenarioEnvironment.hpp"

namespace monadcount_sim::core {
    class ScenarioEnvironmentBuilder {
    public:
        // Build the environment from the parsed Feature objects.
        std::unique_ptr<ScenarioEnvironment> Build(const std::vector<std::unique_ptr<models::Feature>> &features);

    private:
        // Helper methods for each feature type.
        void createApNode(const models::Feature &feature, ScenarioEnvironment &env);

        void createSnifferNode(const models::Feature &feature, ScenarioEnvironment &env);

        void createTerminalNode(const models::Feature &feature, ScenarioEnvironment &env);

        void createObstacle(const models::Feature &feature, ScenarioEnvironment &env);

        void createSeat(const models::Feature &feature, ScenarioEnvironment &env);

        void createDoor(const models::Feature &feature, ScenarioEnvironment &env);
    };
}
#endif //MONADCOUNT_SIM_SCENARIOENVIRONMENTBUILDER_HPP
