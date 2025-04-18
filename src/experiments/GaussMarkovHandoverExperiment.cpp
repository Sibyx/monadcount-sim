// GaussMarkovHandoverExperiment.cpp

#include "GaussMarkovHandoverExperiment.hpp"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"

using namespace ns3;

GaussMarkovHandoverExperiment::GaussMarkovHandoverExperiment()
{
    // Customize parameters for Gauss-Markov mobility if needed.
}

void
GaussMarkovHandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    SetupNodes();
    SetupWifi();
    SetupMobility(); // Overrides base mobility
    SetupInternet();
    SetupApplications();
    SetupTracing();

    Simulator::Schedule(Seconds(1.0), &GaussMarkovHandoverExperiment::CheckRssiAndTriggerHandover, this);

    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
    Simulator::Destroy();
}

void
GaussMarkovHandoverExperiment::SetupMobility()
{
    // APs remain stationary
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(m_wifiApNodes);

    m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(5.0, m_roomWidth / 2.0, 2.0));
    m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(45.0, m_roomWidth / 2.0, 2.0));

    // Pedestrian nodes use Gauss-Markov mobility model
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=50.0]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));

    mobility.SetMobilityModel("ns3::GaussMarkovMobilityModel",
                              "Bounds", BoxValue(Box(0.0, m_roomLength, 0.0, m_roomWidth, 0.0, 5.0)),
                              "TimeStep", TimeValue(Seconds(1.0)),
                              "Alpha", DoubleValue(0.85),
                              "MeanVelocity", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=6.283185]"),
                              "MeanPitch", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
                              "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=1.0|Variance=0.3]"),
                              "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.5]"),
                              "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.05]"));

    mobility.Install(m_groupA);
    mobility.Install(m_groupB);
}

void
GaussMarkovHandoverExperiment::SetupTracing()
{
    #ifdef WITH_NETANIM
    m_anim = new AnimationInterface( "data/gauss-markov-handover/netanim.xml");
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(0)->GetId(), 0, 0, 255);
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(1)->GetId(), 255, 0, 0);
    #endif

    YansWifiPhyHelper wifiPhyHelper;
    for (uint32_t i = 0; i < m_apDevices.GetN(); ++i)
    {
        std::string fileName = "data/gauss-markov-handover/ap_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_apDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
    for (uint32_t i = 0; i < m_staDevices.GetN(); ++i)
    {
        std::string fileName = "data/gauss-markov-handover/sta_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_staDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
}
