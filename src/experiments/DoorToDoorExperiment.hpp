//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
#define MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP

#include "Scenario.hpp"


class DoorToDoorExperiment : public Scenario {
public:
    DoorToDoorExperiment();
    virtual ~DoorToDoorExperiment() = default;
    virtual void Run(ScenarioEnvironment &env) override;
};


#endif //MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
