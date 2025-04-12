//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_PEDESTRIANFACTORY_HPP
#define MONADCOUNT_SIM_PEDESTRIANFACTORY_HPP

#include <ns3/node.h>
#include <monadcount_sim/core/ScenarioEnvironment.hpp>

namespace monadcount_sim::factories::pedestrians {
    class PedestrianFactory {
    public:
        virtual ~PedestrianFactory() = default;

        virtual ns3::Ptr<ns3::Node> Spawn(const core::Door &door, core::ScenarioEnvironment &env) = 0;
    };
}

#endif //MONADCOUNT_SIM_PEDESTRIANFACTORY_HPP
