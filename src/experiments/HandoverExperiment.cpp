#include "HandoverExperiment.hpp"
#include <ns3/animation-interface.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <cmath>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverExperiment");

HandoverExperiment::HandoverExperiment()
        : m_numPedestrians(10),
          m_simulationTime(60.0),
          m_roomLength(50.0),
          m_roomWidth(30.0),
          m_handoverMargin(5.0),
          m_txPower_dBm(20.0),
          m_pathLossExponent(3.0),
          m_anim(nullptr) {}

void HandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env) {
    NS_LOG_INFO("Setting up RSSI-based Handover Experiment...");

    SetupNodes();
    SetupWifi();
    SetupMobility();
    SetupInternet();
    SetupApplications();
    SetupTracing();
    SetupVisualization();

    Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);
    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Handover Simulation complete.");
}

double HandoverExperiment::EstimateRssi(const Vector &stationPos, const Vector &apPos) const {
    double dx = stationPos.x - apPos.x;
    double dy = stationPos.y - apPos.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    if (distance < 1.0) distance = 1.0;
    return m_txPower_dBm - 10.0 * m_pathLossExponent * std::log10(distance);
}

void HandoverExperiment::SetupNodes() {
    m_wifiApNodes.Create(2);
    uint32_t numGroupA = m_numPedestrians / 2;
    uint32_t numGroupB = m_numPedestrians - numGroupA;
    m_groupA.Create(numGroupA);
    m_groupB.Create(numGroupB);

    for (uint32_t i = 0; i < m_groupA.GetN(); ++i) {
        uint32_t nodeId = m_groupA.Get(i)->GetId();
        m_nodeAssociation[nodeId] = 1;
        m_nodeTriggered[nodeId] = false;
    }
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i) {
        uint32_t nodeId = m_groupB.Get(i)->GetId();
        m_nodeAssociation[nodeId] = 2;
        m_nodeTriggered[nodeId] = false;
    }
}

void HandoverExperiment::SetupWifi() {
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    Ssid ssid("eduroam");

    WifiMacHelper macAp;
    macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    m_apDevices = wifi.Install(wifiPhy, macAp, m_wifiApNodes);

    WifiMacHelper macSta;
    macSta.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(true));
    m_staDevices = wifi.Install(wifiPhy, macSta, m_groupA);
    NetDeviceContainer staDevicesB = wifi.Install(wifiPhy, macSta, m_groupB);
    m_staDevices.Add(staDevicesB);

    Ptr<WifiNetDevice> ap1Device = DynamicCast<WifiNetDevice>(m_apDevices.Get(0));
    if (ap1Device) m_ap1Mac = DynamicCast<ApWifiMac>(ap1Device->GetMac());
    Ptr<WifiNetDevice> ap2Device = DynamicCast<WifiNetDevice>(m_apDevices.Get(1));
    if (ap2Device) m_ap2Mac = DynamicCast<ApWifiMac>(ap2Device->GetMac());
}

void HandoverExperiment::SetupMobility() {
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(m_wifiApNodes);
    m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(5.0, m_roomWidth / 2.0, 2.0));
    m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(45.0, m_roomWidth / 2.0, 2.0));

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");

    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));
    mobility.Install(m_groupA);

    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=40.0|Max=50.0]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));
    mobility.Install(m_groupB);

    Ptr<UniformRandomVariable> uvA = CreateObject<UniformRandomVariable>();
    uvA->SetAttribute("Min", DoubleValue(-0.5));
    uvA->SetAttribute("Max", DoubleValue(0.5));
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i) {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupA.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm) cvm->SetVelocity(Vector(1.0, uvA->GetValue(), 0.0));
    }

    Ptr<UniformRandomVariable> uvB = CreateObject<UniformRandomVariable>();
    uvB->SetAttribute("Min", DoubleValue(-0.5));
    uvB->SetAttribute("Max", DoubleValue(0.5));
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i) {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupB.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm) cvm->SetVelocity(Vector(-1.0, uvB->GetValue(), 0.0));
    }
}

void HandoverExperiment::SetupInternet() {
    InternetStackHelper stack;
    stack.Install(m_wifiApNodes);
    stack.Install(m_groupA);
    stack.Install(m_groupB);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(m_apDevices);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void HandoverExperiment::SetupApplications() {
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);

    auto serverApp1 = echoServer.Install(m_wifiApNodes.Get(0));
    auto serverApp2 = echoServer.Install(m_wifiApNodes.Get(1));
    serverApp1.Start(Seconds(0.0));
    serverApp1.Stop(Seconds(m_simulationTime));
    serverApp2.Start(Seconds(0.0));
    serverApp2.Stop(Seconds(m_simulationTime));

    UdpEchoClientHelper echoClient1(Ipv4Address("10.1.1.1"), echoPort);
    echoClient1.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(1024));

    UdpEchoClientHelper echoClient2(Ipv4Address("10.1.1.2"), echoPort);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientAppsA, clientAppsB;
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i)
        clientAppsA.Add(echoClient1.Install(m_groupA.Get(i)));
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i)
        clientAppsB.Add(echoClient2.Install(m_groupB.Get(i)));

    clientAppsA.Start(Seconds(1.0));
    clientAppsA.Stop(Seconds(m_simulationTime));
    clientAppsB.Start(Seconds(1.0));
    clientAppsB.Stop(Seconds(m_simulationTime));
}

void HandoverExperiment::SetupTracing() {
    m_anim = new AnimationInterface("data/handover/netanim.xml");
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(0)->GetId(), 0, 0, 255);
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(1)->GetId(), 255, 0, 0);

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

void HandoverExperiment::RestoreNodeTriggered(uint32_t nodeId) {
    m_nodeTriggered[nodeId] = false;
}

void HandoverExperiment::CheckRssiAndTriggerHandover() {
    Vector ap1Pos = m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->GetPosition();
    Vector ap2Pos = m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->GetPosition();

    NodeContainer allPedestrians;
    allPedestrians.Add(m_groupA);
    allPedestrians.Add(m_groupB);

    for (uint32_t i = 0; i < allPedestrians.GetN(); ++i) {
        Ptr<Node> node = allPedestrians.Get(i);
        uint32_t nodeId = node->GetId();
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        double rssi1 = EstimateRssi(pos, ap1Pos);
        double rssi2 = EstimateRssi(pos, ap2Pos);
        int currentAp = m_nodeAssociation[nodeId];

        if (currentAp == 1 && rssi2 > (rssi1 + m_handoverMargin) && !m_nodeTriggered[nodeId]) {
            m_nodeAssociation[nodeId] = 2;
            m_nodeTriggered[nodeId] = true;
            UpdateNodeVisualColor(nodeId, 2);
            LogHandoverEvent(nodeId, 1, 2, Simulator::Now().GetSeconds());
            Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreNodeTriggered, this, nodeId);
        } else if (currentAp == 2 && rssi1 > (rssi2 + m_handoverMargin) && !m_nodeTriggered[nodeId]) {
            m_nodeAssociation[nodeId] = 1;
            m_nodeTriggered[nodeId] = true;
            UpdateNodeVisualColor(nodeId, 1);
            LogHandoverEvent(nodeId, 2, 1, Simulator::Now().GetSeconds());
            Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreNodeTriggered, this, nodeId);
        } else {
            UpdateNodeVisualColor(nodeId, currentAp);
        }
    }

    if (Simulator::Now().GetSeconds() < m_simulationTime)
        Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);
}

void HandoverExperiment::UpdateNodeVisualColor(uint32_t nodeId, int associatedAp) {
    m_viz.OnNodeAssociated(nodeId, associatedAp);
}

void HandoverExperiment::LogHandoverEvent(uint32_t nodeId, int fromAp, int toAp, double time) {
    m_viz.OnHandoverEvent(nodeId, fromAp, toAp, time);
}

void HandoverExperiment::SetupVisualization() {
    m_viz.EnableNetAnim("data/handover/netanim.xml");
    m_viz.EnableNetSimulyzer("data/handover/simulyzer.json");

    monadcount_sim::core::VisualizationManager::GroupVisualConfig apConfig;
    apConfig.labelPrefix = "AP";
    apConfig.netsimColorFunc = [](uint32_t, int) { return std::make_tuple(0.0, 0.0, 1.0); };
    apConfig.netanimColorFunc = [](uint32_t, int) { return std::make_tuple(0, 0, 255); };
    apConfig.nodeModel = "cell-tower";
    apConfig.enableTrail = false;

    monadcount_sim::core::VisualizationManager::GroupVisualConfig staConfigA;
    staConfigA.labelPrefix = "STA";
    staConfigA.netsimColorFunc = [](uint32_t, int ap) {
        return ap == 1 ? std::make_tuple(0.4, 0.4, 1.0) : std::make_tuple(1.0, 0.6, 0.6);
    };
    staConfigA.netanimColorFunc = [](uint32_t, int ap) {
        return ap == 1 ? std::make_tuple(150, 150, 255) : std::make_tuple(255, 150, 150);
    };
    staConfigA.nodeModel = "person-male";
    staConfigA.enableTrail = true;

    m_viz.RegisterGroup("aps", m_wifiApNodes, apConfig);
    m_viz.RegisterGroup("groupA", m_groupA, staConfigA);
    m_viz.RegisterGroup("groupB", m_groupB, staConfigA);

    m_viz.Initialize();
}
