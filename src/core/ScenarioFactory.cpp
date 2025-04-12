#include <monadcount_sim/core/ScenarioFactory.hpp>

monadcount_sim::core::ScenarioFactory& monadcount_sim::core::ScenarioFactory::Instance() {
    static ScenarioFactory instance;
    return instance;
}

std::unique_ptr<monadcount_sim::core::Scenario> monadcount_sim::core::ScenarioFactory::CreateScenario(const std::string& name) {
    auto it = scenarios.find(name);
    if (it != scenarios.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> monadcount_sim::core::ScenarioFactory::GetAvailableScenarios() const {
    std::vector<std::string> names;
    for (const auto& pair : scenarios) {
        names.push_back(pair.first);
    }
    return names;
}