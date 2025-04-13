#include "ns3/core-module.h"
#include "experiments/BasicExperiment.hpp"
#include "experiments/DoorToDoorExperiment.hpp"
#include "monadcount_sim/core/ScenarioFactory.hpp"
#include "experiments/HandoverExperiment.hpp"
#include "experiments/GaussMarkovHandoverExperiment.hpp"
#include <system_error>

namespace fs = std::filesystem;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MonadCountSim");

void RegisterScenarios() {
    auto& factory = monadcount_sim::core::ScenarioFactory::Instance();
    factory.RegisterScenario<BasicExperiment>("basic");
    factory.RegisterScenario<DoorToDoorExperiment>("doortodoor");
    factory.RegisterScenario<HandoverExperiment>("handover");
    factory.RegisterScenario<GaussMarkovHandoverExperiment>("gauss-markov-handover");
}

int main(int argc, char *argv[])
{
    ns3::LogComponentEnable("MonadCountSim", ns3::LOG_LEVEL_INFO);
    ns3::LogComponentEnable("ScenarioEnvironmentBuilder", ns3::LOG_LEVEL_INFO);

    RegisterScenarios();

    std::string scenarioName = "basic";
    std::string scenarioFile;
    bool listScenarios = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("scenario", "Name of the scenario to run", scenarioName);
    cmd.AddValue("input", "Path to the GeoJSON file describing the scenario (optional)", scenarioFile);
    cmd.AddValue("list-scenarios", "List all available scenario names", listScenarios);
    cmd.Parse(argc, argv);

    auto& factory = monadcount_sim::core::ScenarioFactory::Instance();

    if (listScenarios) {
        for (const auto& name : factory.GetAvailableScenarios()) {
            std::cout << name << std::endl;
        }
        return 0;
    }

    fs::path nestedDir = "data/" + scenarioName;
    try {
        fs::create_directories(nestedDir);
    } catch (const fs::filesystem_error& e) {
        NS_LOG_ERROR("Filesystem error: " << e.what());
        return 1;
    }

    auto scenario = factory.CreateScenario(scenarioName);

    if (!scenario) {
        NS_LOG_ERROR("Unknown scenario: " << scenarioName);
        NS_LOG_INFO("Available scenarios:");
        for (const auto& name : factory.GetAvailableScenarios()) {
            NS_LOG_INFO("  - " << name);
        }
        return 1;
    }

    NS_LOG_INFO("Running scenario: " << scenarioName);
    scenario->Execute(scenarioFile);

    return 0;
}
