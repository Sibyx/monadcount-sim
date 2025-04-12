#include "monadcount_sim/core/Scenario.hpp"
#include "monadcount_sim/core/GeoJsonParser.hpp"
#include "monadcount_sim/core/ScenarioEnvironmentBuilder.hpp"

void monadcount_sim::core::Scenario::Execute(const std::string& scenarioFile) {
    auto env = BuildEnvironment(scenarioFile);
    Run(*env);
}

std::unique_ptr<monadcount_sim::core::ScenarioEnvironment> monadcount_sim::core::Scenario::BuildEnvironment(const std::string& scenarioFile) {
    core::ScenarioEnvironmentBuilder builder;

    if (scenarioFile.empty()) {
        // Create empty environment
        return builder.Build({});
    } else {
        // Parse GeoJSON and build environment
        GeoJSONParser parser;
        auto features = parser.parseFile(scenarioFile);
        return builder.Build(features);
    }
}