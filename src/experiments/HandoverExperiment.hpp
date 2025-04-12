//
// Created by Jakub Dubec on 12/04/2025.
//

#ifndef MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP
#define MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP


#include "monadcount_sim/core/ScenarioEnvironment.hpp"
#include "monadcount_sim/core/Scenario.hpp"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

    class HandoverExperiment : public monadcount_sim::core::Scenario
    {
    public:
        HandoverExperiment ();
        void Run(monadcount_sim::core::ScenarioEnvironment &env);
    private:
        uint32_t m_numPedestrians;
        double m_simulationTime;
        double m_roomLength;
        double m_roomWidth;
    };

#endif //MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP
