// BasicExperiment.hpp
// Created by Jakub Dubec on 02/04/2025.

#ifndef MONADCOUNT_SIM_BASICEXPERIMENT_HPP
#define MONADCOUNT_SIM_BASICEXPERIMENT_HPP

#include "Scenario.hpp"
#include "ns3/core-module.h"

class BasicExperiment : public Scenario {
public:
    BasicExperiment();
    virtual ~BasicExperiment() = default;
    virtual void Run(ScenarioEnvironment &env) override;

private:
    uint32_t m_numPedestrians;
    double m_simulationTime;
    double m_roomLength;
    double m_roomWidth;
};

#endif //MONADCOUNT_SIM_BASICEXPERIMENT_HPP
