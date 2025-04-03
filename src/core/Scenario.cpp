#include "Scenario.hpp"
#include "GeoJsonParser.hpp"
#include "ScenarioEnvironmentBuilder.hpp"

void Scenario::Execute(const std::string& scenarioFile) {
    auto env = BuildEnvironment(scenarioFile);
    Run(*env);
}

std::unique_ptr<ScenarioEnvironment> Scenario::BuildEnvironment(const std::string& scenarioFile) {
    ScenarioEnvironmentBuilder builder;

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