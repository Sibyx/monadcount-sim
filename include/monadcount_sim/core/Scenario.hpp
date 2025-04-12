#ifndef MONADCOUNT_SIM_SCENARIO_HPP
#define MONADCOUNT_SIM_SCENARIO_HPP

#include <ns3/core-module.h>
#include <memory>
#include <string>
#include "ScenarioEnvironment.hpp"

namespace monadcount_sim::core {
    class Scenario {
    public:
        virtual ~Scenario() = default;

        // Main entry point that automatically builds environment if needed
        void Execute(const std::string &scenarioFile = "");

        // Override this to customize environment building if needed
        virtual std::unique_ptr<ScenarioEnvironment> BuildEnvironment(const std::string &scenarioFile);

    protected:
        // Actual simulation implementation
        virtual void Run(ScenarioEnvironment &env) = 0;
    };
}

#endif //MONADCOUNT_SIM_SCENARIO_HPP