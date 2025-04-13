#include "GaussMarkovHandoverExperiment.hpp"
#include <ns3/mobility-module.h>
#include <ns3/simulator.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GaussMarkovHandoverExperiment");

GaussMarkovHandoverExperiment::GaussMarkovHandoverExperiment()
{
    // Optionally tweak experiment-specific parameters
}

void GaussMarkovHandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    NS_LOG_INFO("Setting up Gauss-Markov Handover Experiment...");

    SetupNodes();
    SetupWifi();
    SetupMobility();  // overridden
    SetupInternet();
    SetupApplications();
    SetupTracing();
    SetupVisualization();

    Simulator::Schedule(Seconds(1.0), &GaussMarkovHandoverExperiment::CheckRssiAndTriggerHandover, this);

    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Gauss-Markov Handover Simulation complete.");
}

void GaussMarkovHandoverExperiment::SetupMobility()
{
    // (a) APs are stationary
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(m_wifiApNodes);

    m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(5.0, m_roomWidth / 2.0, 2.0));
    m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(45.0, m_roomWidth / 2.0, 2.0));

    // (b) Pedestrians use Gauss-Markov mobility
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

void GaussMarkovHandoverExperiment::SetupTracing() {
    YansWifiPhyHelper wifiPhyHelper;
    for (uint32_t i = 0; i < m_apDevices.GetN(); ++i) {
        std::string fileName = "data/handover/ap_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_apDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
    for (uint32_t i = 0; i < m_staDevices.GetN(); ++i) {
        std::string fileName = "data/handover/sta_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_staDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
}

void GaussMarkovHandoverExperiment::SetupVisualization()
{
    m_viz.EnableNetAnim("data/gauss-markov-handover/netanim.xml");
    m_viz.EnableNetSimulyzer("data/gauss-markov-handover/simulyzer.json");

    monadcount_sim::core::VisualizationManager::GroupVisualConfig apConfig;
    apConfig.labelPrefix = "AP";
    apConfig.netsimColorFunc = [](uint32_t, int) { return std::make_tuple(0.0, 0.0, 1.0); };
    apConfig.netanimColorFunc = [](uint32_t, int) { return std::make_tuple(0, 0, 255); };
    apConfig.nodeModel = "cell-tower";
    apConfig.enableTrail = false;

    monadcount_sim::core::VisualizationManager::GroupVisualConfig staConfig;
    staConfig.labelPrefix = "STA";
    staConfig.netsimColorFunc = [](uint32_t, int ap) {
        return ap == 1 ? std::make_tuple(0.4, 0.4, 1.0) : std::make_tuple(1.0, 0.6, 0.6);
    };
    staConfig.netanimColorFunc = [](uint32_t, int ap) {
        return ap == 1 ? std::make_tuple(150, 150, 255) : std::make_tuple(255, 150, 150);
    };
    staConfig.nodeModel = "person-male";
    staConfig.enableTrail = true;

    m_viz.RegisterGroup("aps", m_wifiApNodes, apConfig);
    m_viz.RegisterGroup("groupA", m_groupA, staConfig);
    m_viz.RegisterGroup("groupB", m_groupB, staConfig);

    m_viz.Initialize();
}
