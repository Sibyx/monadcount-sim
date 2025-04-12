#include "HandoverExperiment.hpp"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <cmath>
#include <string>
#include <map>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverExperiment");

HandoverExperiment::HandoverExperiment ()
        : m_numPedestrians(10),
          m_simulationTime(60.0),
          m_roomLength(50.0),
          m_roomWidth(30.0),
          m_handoverMargin(5.0),
          m_txPower_dBm(20.0),
          m_pathLossExponent(3.0),
          m_anim(nullptr)
{
    // The maps m_nodeAssociation and m_nodeTriggered are initialized later in SetupNodes.
}

void
HandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    NS_LOG_INFO("Setting up RSSI-based Handover Experiment...");

    SetupNodes();
    SetupWifi();
    SetupMobility();
    SetupInternet();
    SetupApplications();
    SetupTracing();

    // Schedule the periodic per-node handover and color update.
    Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);

    Simulator::Stop(Seconds(m_simulationTime));
    NS_LOG_INFO("Running Handover Simulation...");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Handover Simulation complete.");
}

double
HandoverExperiment::EstimateRssi(const Vector &stationPos, const Vector &apPos) const
{
    double dx = stationPos.x - apPos.x;
    double dy = stationPos.y - apPos.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    if (distance < 1.0)
    {
        distance = 1.0;
    }
    return m_txPower_dBm - 10.0 * m_pathLossExponent * std::log10(distance);
}

void
HandoverExperiment::SetupNodes()
{
    // Create AP nodes.
    m_wifiApNodes.Create(2);

    // Divide pedestrians into two groups.
    uint32_t numGroupA = m_numPedestrians / 2;
    uint32_t numGroupB = m_numPedestrians - numGroupA;
    m_groupA.Create(numGroupA);
    m_groupB.Create(numGroupB);

    // Initialize per-node association and triggered maps.
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i) {
        uint32_t nodeId = m_groupA.Get(i)->GetId();
        m_nodeAssociation[nodeId] = 1; // Group A initially with AP1.
        m_nodeTriggered[nodeId] = false;
    }
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i) {
        uint32_t nodeId = m_groupB.Get(i)->GetId();
        m_nodeAssociation[nodeId] = 2; // Group B initially with AP2.
        m_nodeTriggered[nodeId] = false;
    }
}

void
HandoverExperiment::SetupWifi()
{
    // Create a single Wi-Fi channel with delay and propagation loss models.
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    Ssid ssid("eduroam");

    // Install Wi-Fi devices on the AP nodes.
    WifiMacHelper macAp;
    macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    m_apDevices = wifi.Install(wifiPhy, macAp, m_wifiApNodes);

    // Install Wi-Fi devices on pedestrian nodes.
    WifiMacHelper macSta;
    macSta.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(true));
    m_staDevices = wifi.Install(wifiPhy, macSta, m_groupA);
    NetDeviceContainer staDevicesB = wifi.Install(wifiPhy, macSta, m_groupB);
    m_staDevices.Add(staDevicesB);

    // Retrieve pointers to the AP MAC objects.
    Ptr<WifiNetDevice> ap1Device = DynamicCast<WifiNetDevice>(m_apDevices.Get(0));
    if (ap1Device)
    {
        m_ap1Mac = DynamicCast<ApWifiMac>(ap1Device->GetMac());
    }
    Ptr<WifiNetDevice> ap2Device = DynamicCast<WifiNetDevice>(m_apDevices.Get(1));
    if (ap2Device)
    {
        m_ap2Mac = DynamicCast<ApWifiMac>(ap2Device->GetMac());
    }
}

void
HandoverExperiment::SetupMobility()
{
    // (a) Set AP positions at opposite sides.
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(m_wifiApNodes);
    Ptr<MobilityModel> apMob1 = m_wifiApNodes.Get(0)->GetObject<MobilityModel>();
    apMob1->SetPosition(Vector(5.0, m_roomWidth / 2.0, 2.0)); // AP1 near left.
    Ptr<MobilityModel> apMob2 = m_wifiApNodes.Get(1)->GetObject<MobilityModel>();
    apMob2->SetPosition(Vector(45.0, m_roomWidth / 2.0, 2.0)); // AP2 near right.

    // (b) Set pedestrian mobility.
    // Group A: Starting at the left end, moving to the right.
    MobilityHelper mobilityGroupA;
    mobilityGroupA.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityGroupA.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                        "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"),
                                        "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));
    mobilityGroupA.Install(m_groupA);

    // Group B: Starting at the right end, moving to the left.
    MobilityHelper mobilityGroupB;
    mobilityGroupB.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityGroupB.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                        "X", StringValue("ns3::UniformRandomVariable[Min=40.0|Max=50.0]"),
                                        "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));
    mobilityGroupB.Install(m_groupB);

    // Create a uniform random variable stream for the y-axis velocity.
    Ptr<UniformRandomVariable> uvA = CreateObject<UniformRandomVariable> ();
    uvA->SetAttribute("Min", DoubleValue(-0.5));
    uvA->SetAttribute("Max", DoubleValue(0.5));

    Ptr<UniformRandomVariable> uvB = CreateObject<UniformRandomVariable> ();
    uvB->SetAttribute("Min", DoubleValue(-0.5));
    uvB->SetAttribute("Max", DoubleValue(0.5));

    // Set individual velocities for Group A with random y component.
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupA.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm)
        {
            double vy = uvA->GetValue(); // Random y velocity in [-0.5, 0.5]
            // For Group A, x-velocity is 1.0 (moving right).
            cvm->SetVelocity(Vector(1.0, vy, 0.0));
        }
    }

    // Set individual velocities for Group B with random y component.
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupB.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm)
        {
            double vy = uvB->GetValue(); // Random y velocity in [-0.5, 0.5]
            // For Group B, x-velocity is -1.0 (moving left).
            cvm->SetVelocity(Vector(-1.0, vy, 0.0));
        }
    }
}

void
HandoverExperiment::SetupInternet()
{
    InternetStackHelper stack;
    stack.Install(m_wifiApNodes);
    stack.Install(m_groupA);
    stack.Install(m_groupB);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer apInterfaces = address.Assign(m_apDevices);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void
HandoverExperiment::SetupApplications()
{
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);

    ApplicationContainer serverApp1 = echoServer.Install(m_wifiApNodes.Get(0));
    ApplicationContainer serverApp2 = echoServer.Install(m_wifiApNodes.Get(1));
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
    {
        clientAppsA.Add(echoClient1.Install(m_groupA.Get(i)));
    }
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i)
    {
        clientAppsB.Add(echoClient2.Install(m_groupB.Get(i)));
    }
    clientAppsA.Start(Seconds(1.0));
    clientAppsA.Stop(Seconds(m_simulationTime));
    clientAppsB.Start(Seconds(1.0));
    clientAppsB.Stop(Seconds(m_simulationTime));
}

void
HandoverExperiment::SetupTracing()
{
    // Create NetAnim XML output.
    m_anim = new AnimationInterface("data/handover/netanim.xml");
    // Fixed colors for AP nodes.
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(0)->GetId(), 0, 0, 255);   // Dark blue.
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(1)->GetId(), 255, 0, 0);   // Dark red.

    // Enable PCAP generation.
    YansWifiPhyHelper wifiPhyHelper;
    for (uint32_t i = 0; i < m_apDevices.GetN(); ++i)
    {
        std::string fileName = "data/handover/ap_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_apDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
    for (uint32_t i = 0; i < m_staDevices.GetN(); ++i)
    {
        std::string fileName = "data/handover/sta_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_staDevices.Get(i), true, YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
}

void
HandoverExperiment::RestoreNodeTriggered(uint32_t nodeId)
{
    m_nodeTriggered[nodeId] = false;
}

void
HandoverExperiment::CheckRssiAndTriggerHandover()
{
    // Retrieve AP positions.
    Vector ap1Pos = m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->GetPosition();
    Vector ap2Pos = m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->GetPosition();

    // Combine pedestrian nodes.
    NodeContainer allPedestrians;
    allPedestrians.Add(m_groupA);
    allPedestrians.Add(m_groupB);

    for (uint32_t i = 0; i < allPedestrians.GetN(); ++i)
    {
        Ptr<Node> node = allPedestrians.Get(i);
        uint32_t nodeId = node->GetId();
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        double rssi1 = EstimateRssi(pos, ap1Pos);
        double rssi2 = EstimateRssi(pos, ap2Pos);
        int currentAssociation = m_nodeAssociation[nodeId];

        if (currentAssociation == 1)
        {
            // Currently associated with AP1.
            if (rssi2 > (rssi1 + m_handoverMargin) && !m_nodeTriggered[nodeId])
            {
                NS_LOG_INFO("Node " << nodeId << " handing over from AP1 to AP2.");
                m_nodeAssociation[nodeId] = 2;
                m_nodeTriggered[nodeId] = true;
                m_anim->UpdateNodeColor(nodeId, 255, 150, 150); // Light red.
                Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreNodeTriggered, this, nodeId);
            }
            else
            {
                m_anim->UpdateNodeColor(nodeId, 150, 150, 255); // Ensure light blue if still with AP1.
            }
        }
        else if (currentAssociation == 2)
        {
            // Currently associated with AP2.
            if (rssi1 > (rssi2 + m_handoverMargin) && !m_nodeTriggered[nodeId])
            {
                NS_LOG_INFO("Node " << nodeId << " handing over from AP2 to AP1.");
                m_nodeAssociation[nodeId] = 1;
                m_nodeTriggered[nodeId] = true;
                m_anim->UpdateNodeColor(nodeId, 150, 150, 255); // Light blue.
                Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreNodeTriggered, this, nodeId);
            }
            else
            {
                m_anim->UpdateNodeColor(nodeId, 255, 150, 150); // Ensure light red if still with AP2.
            }
        }
    }

    // Reschedule check every second.
    if (Simulator::Now().GetSeconds() < m_simulationTime)
    {
        Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);
    }
}
