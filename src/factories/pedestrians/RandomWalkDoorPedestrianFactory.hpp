//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP
#define MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP

#include "PedestrianFactory.hpp"

class RandomWalkDoorPedestrianFactory : public PedestrianFactory {
public:
    virtual ns3::Ptr<ns3::Node> Spawn(const Door &door, ScenarioEnvironment &env) override;
};


#endif //MONADCOUNT_SIM_RANDOMWALKDOORPEDESTRIANFACTORY_HPP
