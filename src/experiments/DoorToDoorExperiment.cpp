#include "DoorToDoorExperiment.hpp"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ScenarioEnvironment.hpp"

NS_LOG_COMPONENT_DEFINE ("DoorToDoorExperiment");

DoorToDoorExperiment::DoorToDoorExperiment()
        : m_simulationTime(20.0) {
    // Initialization if needed
}

void DoorToDoorExperiment::Run(ScenarioEnvironment &env)
{
    NS_LOG_INFO("Running Experiment: Door-to-Door");

    // Create a factory to spawn door-to-door pedestrians.
    // DoorToDoorPedestrianFactory factory;

    // For each door in the environment, spawn a pedestrian.
    for (auto &door : env.doors)
    {
        NS_LOG_INFO("Found door at position (" << door.x << ", "
                                               << door.y << ")");

        // Schedule an immediate spawn event for each door.
        // ns3::Simulator::Schedule(ns3::Seconds(0.0),
        //                         &DoorToDoorPedestrianFactory::SpawnPedestrian,
        //                         &factory, door, std::ref(env));
    }

    // Run the simulation for a fixed duration.
    ns3::Simulator::Stop(ns3::Seconds(m_simulationTime));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    NS_LOG_INFO("Door-to-Door experiment complete.");
}