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

class HandoverExperiment : public monadcount_sim::core::Scenario
{
public:
    HandoverExperiment ();
    virtual void Run(monadcount_sim::core::ScenarioEnvironment &env);

protected:
    // Simulation parameters.
    uint32_t m_numPedestrians;
    double m_simulationTime;
    double m_roomLength;
    double m_roomWidth;

    // Additional simulation parameters.
    double m_handoverMargin;
    double m_txPower_dBm;
    double m_pathLossExponent;

    // Node containers for APs and pedestrian groups.
    ns3::NodeContainer m_wifiApNodes;
    ns3::NodeContainer m_groupA;
    ns3::NodeContainer m_groupB;

    // NetDevice containers to store installed devices.
    ns3::NetDeviceContainer m_apDevices;
    ns3::NetDeviceContainer m_staDevices;

    // Pointers to the AP MAC objects.
    ns3::Ptr<ns3::ApWifiMac> m_ap1Mac;
    ns3::Ptr<ns3::ApWifiMac> m_ap2Mac;

    // Handover trigger flags.
    bool m_handoverTriggeredAP1;
    bool m_handoverTriggeredAP2;

    std::map<uint32_t, int> m_nodeAssociation;
    std::map<uint32_t, bool> m_nodeTriggered;


    // Animation interface pointer for NetAnim.
    ns3::AnimationInterface* m_anim;

    // Private helper functions.
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
