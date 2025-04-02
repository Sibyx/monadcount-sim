//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_SCENARIO_HPP
#define MONADCOUNT_SIM_SCENARIO_HPP


#include "ns3/core-module.h"
#include "ScenarioEnvironment.hpp"

class Scenario {
public:
    virtual ~Scenario() = default;
    virtual void Run(ScenarioEnvironment &env) = 0;
};


#endif //MONADCOUNT_SIM_SCENARIO_HPP
