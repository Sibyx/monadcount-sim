#ifndef MONADCOUNT_SIM_SCENARIOFACTORY_HPP
#define MONADCOUNT_SIM_SCENARIOFACTORY_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "Scenario.hpp"

namespace monadcount_sim::core {
    class ScenarioFactory {
    public:
        static ScenarioFactory &Instance();

        // Register a scenario type with the factory
        template<typename T>
        void RegisterScenario(const std::string &name) {
            scenarios[name] = []() { return std::make_unique<T>(); };
        }

        // Create a scenario by name
        std::unique_ptr<Scenario> CreateScenario(const std::string &name);

        // Get available scenario names
        std::vector<std::string> GetAvailableScenarios() const;

    private:
        ScenarioFactory() = default;

        std::unordered_map<std::string, std::function<std::unique_ptr<Scenario>()>> scenarios;
    };
}

#endif //MONADCOUNT_SIM_SCENARIOFACTORY_HPP