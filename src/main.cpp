#include "ns3/core-module.h"
#include "GeoJsonParser.hpp"
#include "ScenarioEnvironmentBuilder.hpp"
#include "experiments/BasicExperiment.hpp"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MonadCountSim");

int main(int argc, char *argv[])
{
    ns3::LogComponentEnable("MonadCountSim", ns3::LOG_LEVEL_INFO);
    ns3::LogComponentEnable("ScenarioEnvironmentBuilder", ns3::LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    std::string scenarioFile = "geojson/room.geo.json"; // default GeoJSON path
    cmd.AddValue("scenario", "Path to the GeoJSON file describing the scenario", scenarioFile);
    cmd.Parse(argc, argv);

    NS_LOG_INFO ("Starting simulation environment build...");

    GeoJSONParser parser;
    auto features = parser.parseFile(scenarioFile);

    ScenarioEnvironmentBuilder builder;
    std::unique_ptr<ScenarioEnvironment> env = builder.Build(features);

    // 3. Log environment details.
//    NS_LOG_INFO ("AP count: " << env->apNodes.GetN());
//    NS_LOG_INFO ("Sniffer count: " << env->snifferNodes.GetN());
//    NS_LOG_INFO ("Terminal count: " << env->terminalNodes.GetN());
//    NS_LOG_INFO ("Obstacles: " << env->obstacles.size());
//    NS_LOG_INFO ("Seats: " << env->seats.size());
//    NS_LOG_INFO ("Doors: " << env->doors.size());

//    ns3::Simulator::Run();
//    ns3::Simulator::Destroy();

    BasicExperiment experiment;
    experiment.Run(env);

    return 0;
}
