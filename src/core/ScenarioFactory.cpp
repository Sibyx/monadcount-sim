#include "ScenarioFactory.hpp"

ScenarioFactory& ScenarioFactory::Instance() {
    static ScenarioFactory instance;
    return instance;
}

std::unique_ptr<Scenario> ScenarioFactory::CreateScenario(const std::string& name) {
    auto it = scenarios.find(name);
    if (it != scenarios.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> ScenarioFactory::GetAvailableScenarios() const {
    std::vector<std::string> names;
    for (const auto& pair : scenarios) {
        names.push_back(pair.first);
    }
    return names;
}