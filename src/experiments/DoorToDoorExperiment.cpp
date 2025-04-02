#include "DoorToDoorExperiment.hpp"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ScenarioEnvironment.hpp"

NS_LOG_COMPONENT_DEFINE ("DoorToDoorExperiment");

DoorToDoorExperiment::DoorToDoorExperiment() {
    // Initialization, if needed.
}

void DoorToDoorExperiment::Run(ScenarioEnvironment &env)
{
    NS_LOG_INFO ("Running Experiment: Door-to-Door");

    // Create a factory to spawn door-to-door pedestrians.
//    DoorToDoorPedestrianFactory factory;

    // For each door in the environment, spawn a pedestrian.
    for (auto &door : env.doors)
    {
        // Schedule an immediate spawn event for each door.
//        ns3::Simulator::Schedule(ns3::Seconds(0.0),
//                                 &DoorToDoorPedestrianFactory::SpawnPedestrian,
//                                 &factory, door, std::ref(env));
    }

    // In a full experiment you would also set up callbacks or events
    // so that when a pedestrian "finds" another door, it is removed (dies)
    // and a new pedestrian is spawned from that door.

    // Run the simulation for a fixed duration.
    ns3::Simulator::Stop(ns3::Seconds(20.0));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}
