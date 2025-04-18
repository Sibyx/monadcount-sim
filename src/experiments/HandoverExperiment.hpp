//
// Created by Jakub Dubec on 12/04/2025.
//

#ifndef MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP
#define MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP

#include <ns3/animation-interface.h>
#include "monadcount_sim/core/ScenarioEnvironment.hpp"
#include "monadcount_sim/core/Scenario.hpp"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include <map>
#include <vector>
#include <string>

class HandoverExperiment : public monadcount_sim::core::Scenario
{
public:
    HandoverExperiment ();
    virtual void Run(monadcount_sim::core::ScenarioEnvironment &env);

protected:
    // Configurable parameters
    uint32_t            m_numAp;
    uint32_t            m_numPedestrians;
    double              m_simulationTime;
    double              m_roomLength;
    double              m_roomWidth;
    double              m_handoverMargin;
    double              m_txPower_dBm;
    double              m_pathLossExponent;
    std::string         m_mobilityModel;

    // Nodes & devices
    ns3::NodeContainer              m_wifiApNodes;
    std::vector<ns3::NodeContainer> m_groups;
    ns3::NetDeviceContainer         m_apDevices;
    ns3::NetDeviceContainer         m_staDevices;

    // Handover state
    std::map<uint32_t,int>      m_nodeAssociation;
    std::map<uint32_t,bool>     m_nodeTriggered;
    std::map<uint32_t,uint32_t> m_handoverCount;

    // NetAnim
    ns3::AnimationInterface*     m_anim;

    // Helpers
    double EstimateRssi(const ns3::Vector &stationPos, const ns3::Vector &apPos) const;
    void SetupNodes();
    void SetupWifi();
    void SetupMobility();
    void SetupInternet();
    void SetupApplications();
    void SetupTracing();
    void RestoreNodeTriggered(uint32_t nodeId);
    void CheckRssiAndTriggerHandover();
};

#endif // MONADCOUNT_SIM_HANDOVEREXPERIMENT_HPP
