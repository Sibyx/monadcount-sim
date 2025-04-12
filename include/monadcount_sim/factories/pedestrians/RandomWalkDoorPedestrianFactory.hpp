//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP
#define MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP

#include <monadcount_sim/core/ScenarioEnvironment.hpp>

#include "PedestrianFactory.hpp"

namespace monadcount_sim::factories::pedestrians {
    class RandomWalkDoorPedestrianFactory : public PedestrianFactory {
    public:
        virtual ns3::Ptr<ns3::Node> Spawn(const core::Door &door, core::ScenarioEnvironment &env) override;
    };
}


#endif //MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP
