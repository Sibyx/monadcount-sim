//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_SCENARIOENVIRONMENT_HPP
#define MONADCOUNT_SIM_SCENARIOENVIRONMENT_HPP


#include <ns3/node-container.h>
#include <ns3/ptr.h>
#include <ns3/propagation-loss-model.h>
#include <vector>
#include <string>

namespace monadcount_sim::core {
    // Obstacle represents a wall, table, or other signal-affecting object.
    struct Obstacle {
        std::string id;
        // Additional geometry or bounding box information can be added here.
    };

    // Seat is an obstacle that can be occupied.
    struct Seat {
        std::string id;
        bool occupied;
        double x;
        double y;

        Seat() : occupied(false), x(0.0), y(0.0) {}
    };

    // Door is a spawn point for pedestrians.
    struct Door {
        std::string id;
        double x;
        double y;

        Door() : x(0.0), y(0.0) {}
    };

    class ScenarioEnvironment {
    public:
        // NodeContainers for different device types.
        ns3::NodeContainer apNodes;       // Access Points
        ns3::NodeContainer snifferNodes;  // Sniffers
        ns3::NodeContainer terminalNodes; // Terminals (static or initial)

        // Collections for dynamic actors (obstacles, seats, doors)
        std::vector<Obstacle> obstacles;
        std::vector<Seat> seats;
        std::vector<Door> doors;

        // Optionally, a pointer to a custom PropagationLossModel
        ns3::Ptr<ns3::PropagationLossModel> obstacleLossModel;
    };

}

#endif //MONADCOUNT_SIM_SCENARIOENVIRONMENT_HPP
