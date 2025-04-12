#include <monadcount_sim/core/ScenarioEnvironmentBuilder.hpp>
#include "ns3/node.h"
#include "ns3/mobility-helper.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "monadcount_sim/models/PointGeometry.hpp"


NS_LOG_COMPONENT_DEFINE ("ScenarioEnvironmentBuilder");

std::unique_ptr<monadcount_sim::core::ScenarioEnvironment> monadcount_sim::core::ScenarioEnvironmentBuilder::Build(const std::vector<std::unique_ptr<monadcount_sim::models::Feature>> &features)
{
    NS_LOG_INFO ("Building NS-3 Environment from Features...");
    auto env = std::make_unique<ScenarioEnvironment>();

    // Loop over each Feature and build the corresponding ns-3 object or dynamic actor.
    for (const auto &f : features)
    {
        switch (f->getCategory().getType())
        {
            case models::Category::ACCESS_POINT:
                createApNode(*f, *env);
                break;
            case models::Category::SNIFFER:
                createSnifferNode(*f, *env);
                break;
            case models::Category::TERMINAL:
                createTerminalNode(*f, *env);
                break;
            case models::Category::WALL:
            case models::Category::TABLE:
                createObstacle(*f, *env);
                break;
            case models::Category::SEAT:
                createSeat(*f, *env);
                break;
            case models::Category::DOOR:
                createDoor(*f, *env);
                break;
            default:
                NS_LOG_WARN ("Unhandled feature category: " << f->getCategory().toString());
                break;
        }
    }

    NS_LOG_INFO ("Environment build complete.");
    return env;
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createApNode(const models::Feature &feature, ScenarioEnvironment &env)
{
    ns3::Ptr<ns3::Node> node = ns3::CreateObject<ns3::Node>();

    // If the feature has a point geometry, use it to set the node position.
    if (feature.getGeometry() && feature.getGeometry()->getType() == "Point")
    {
        auto pt = dynamic_cast<const models::PointGeometry*>(feature.getGeometry());
        if (pt)
        {
            ns3::Ptr<ns3::ListPositionAllocator> posAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
            posAlloc->Add(ns3::Vector(pt->point.x, pt->point.y, 0.0));

            ns3::MobilityHelper mobility;
            mobility.SetPositionAllocator(posAlloc);
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(node);
            NS_LOG_DEBUG ("AP node positioned at (" << pt->point.x << ", " << pt->point.y << ")");
        }
    }

    // Installation of Wi-Fi (or BLE) devices is omitted for brevity.
    env.apNodes.Add(node);
    NS_LOG_DEBUG ("AP node created. Total APs: " << env.apNodes.GetN());
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createSnifferNode(const models::Feature &feature, ScenarioEnvironment &env)
{
    ns3::Ptr<ns3::Node> node = ns3::CreateObject<ns3::Node>();

    if (feature.getGeometry() && feature.getGeometry()->getType() == "Point")
    {
        auto pt = dynamic_cast<const models::PointGeometry*>(feature.getGeometry());
        if (pt)
        {
            ns3::Ptr<ns3::ListPositionAllocator> posAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
            posAlloc->Add(ns3::Vector(pt->point.x, pt->point.y, 0.0));

            ns3::MobilityHelper mobility;
            mobility.SetPositionAllocator(posAlloc);
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(node);

            NS_LOG_DEBUG ("Sniffer node positioned at (" << pt->point.x << ", " << pt->point.y << ")");
        }
    }

    // Attach multiple network devices (e.g., two BLE cards) as required.
    env.snifferNodes.Add(node);
    NS_LOG_DEBUG ("Sniffer node created. Total sniffers: " << env.snifferNodes.GetN());
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createTerminalNode(const models::Feature &feature, ScenarioEnvironment &env)
{
    ns3::Ptr<ns3::Node> node = ns3::CreateObject<ns3::Node>();

    if (feature.getGeometry() && feature.getGeometry()->getType() == "Point")
    {
        auto pt = dynamic_cast<const models::PointGeometry*>(feature.getGeometry());
        if (pt)
        {
            ns3::Ptr<ns3::ListPositionAllocator> posAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
            posAlloc->Add(ns3::Vector(pt->point.x, pt->point.y, 0.0));

            ns3::MobilityHelper mobility;
            mobility.SetPositionAllocator(posAlloc);
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(node);

            NS_LOG_DEBUG ("Terminal node positioned at (" << pt->point.x << ", " << pt->point.y << ")");
        }
    }

    // Installation of terminal network devices in station mode is omitted.
    env.terminalNodes.Add(node);
    NS_LOG_DEBUG ("Terminal node created. Total terminals: " << env.terminalNodes.GetN());
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createObstacle(const models::Feature &feature, ScenarioEnvironment &env)
{
    Obstacle obs;
    obs.id = feature.getId();
    // Additional geometry processing can be added here.
    env.obstacles.push_back(obs);
    NS_LOG_DEBUG ("Obstacle created with id " << obs.id << ". Total obstacles: " << env.obstacles.size());
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createSeat(const models::Feature &feature, ScenarioEnvironment &env)
{
    Seat seat;
    seat.id = feature.getId();

    if (feature.getGeometry() && feature.getGeometry()->getType() == "Point")
    {
        auto pt = dynamic_cast<const models::PointGeometry*>(feature.getGeometry());
        if (pt)
        {
            seat.x = pt->point.x;
            seat.y = pt->point.y;
            NS_LOG_DEBUG("Seat positioned at (" << seat.x << ", " << seat.y << ")");
        }
    }

    env.seats.push_back(seat);
    NS_LOG_DEBUG ("Seat created with id " << seat.id << ". Total seats: " << env.seats.size());
}

void monadcount_sim::core::ScenarioEnvironmentBuilder::createDoor(const models::Feature &feature, ScenarioEnvironment &env)
{
    Door door;
    door.id = feature.getId();

    if (feature.getGeometry() && feature.getGeometry()->getType() == "Point")
    {
        auto pt = dynamic_cast<const models::PointGeometry*>(feature.getGeometry());
        if (pt)
        {
            door.x = pt->point.x;
            door.y = pt->point.y;
            NS_LOG_DEBUG ("Door positioned at (" << door.x << ", " << door.y << ")");
        }
    }

    env.doors.push_back(door);
    NS_LOG_DEBUG ("Door created with id " << door.id << ". Total doors: " << env.doors.size());
}
