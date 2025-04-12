//
// Created by Jakub Dubec on 01/04/2025.
//

#include "ns3/node.h"
#include "ns3/mobility-helper.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "monadcount_sim/factories/pedestrians/RandomWalkDoorPedestrianFactory.hpp"

NS_LOG_COMPONENT_DEFINE ("RandomWalkDoorPedestrianFactory");

ns3::Ptr<ns3::Node> monadcount_sim::factories::pedestrians::RandomWalkDoorPedestrianFactory::Spawn(const monadcount_sim::core::Door &door,
                                                                 monadcount_sim::core::ScenarioEnvironment &env)
{
    ns3::Ptr<ns3::Node> node = ns3::CreateObject<ns3::Node>();

    // Set the initial position at the door.
    ns3::Ptr<ns3::ListPositionAllocator> posAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
    posAlloc->Add(ns3::Vector(door.x, door.y, 0.0));

    ns3::MobilityHelper mobility;
    mobility.SetPositionAllocator(posAlloc);
    // For a door-to-door scenario, we use a mobility model that makes the pedestrian wander.
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
    mobility.Install(node);

    NS_LOG_INFO ("Spawned pedestrian at door " << door.id);

    // In a complete implementation, you would attach a terminal (with an echo server) to the node.
    // You might also register event callbacks so that when the pedestrian finds a door, it is removed
    // and the door spawns a new pedestrian.

    return node;
}
