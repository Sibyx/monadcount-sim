#include "HandoverExperiment.hpp"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <cmath>
#include <string>

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
          m_handoverTriggeredAP1(false),
          m_handoverTriggeredAP2(false),
          m_anim(nullptr)
{
}

void
HandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    NS_LOG_INFO("Setting up reimplemented RSSI-based Handover Experiment...");

    SetupNodes();
    SetupWifi();
    SetupMobility();
    SetupInternet();
    SetupApplications();
    SetupTracing();

    // Schedule the periodic RSSI evaluation and potential handover trigger.
    Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);

    Simulator::Stop(Seconds(m_simulationTime));
    NS_LOG_INFO("Running reimplemented Handover Simulation...");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Reimplemented Handover Simulation complete.");
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
    // (a) Static AP positions.
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(m_wifiApNodes);
    Ptr<MobilityModel> apMob1 = m_wifiApNodes.Get(0)->GetObject<MobilityModel>();
    apMob1->SetPosition(Vector(10.0, m_roomWidth / 2.0, 2.0));
    Ptr<MobilityModel> apMob2 = m_wifiApNodes.Get(1)->GetObject<MobilityModel>();
    apMob2->SetPosition(Vector(40.0, m_roomWidth / 2.0, 2.0));

    // (b) Pedestrian mobility.
    // Group A: Moving from left to right.
    MobilityHelper mobilityGroupA;
    mobilityGroupA.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityGroupA.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                        "X", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=15.0]"),
                                        "Y", StringValue("ns3::UniformRandomVariable[Min=10.0|Max=20.0]"));
    mobilityGroupA.Install(m_groupA);

    // Group B: Moving from right to left.
    MobilityHelper mobilityGroupB;
    mobilityGroupB.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityGroupB.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                        "X", StringValue("ns3::UniformRandomVariable[Min=35.0|Max=45.0]"),
                                        "Y", StringValue("ns3::UniformRandomVariable[Min=10.0|Max=20.0]"));
    mobilityGroupB.Install(m_groupB);

    // Set initial velocities with slight vertical variations.
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupA.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm)
        {
            cvm->SetVelocity(Vector(1.0, 0.2 * ((i % 3) - 1), 0.0)); // Rightward
        }
    }
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> cvm = m_groupB.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        if (cvm)
        {
            cvm->SetVelocity(Vector(-1.0, 0.2 * ((i % 3) - 1), 0.0)); // Leftward
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

    // Assign IP addresses to AP nodes using the stored NetDeviceContainer.
    Ipv4InterfaceContainer apInterfaces = address.Assign(m_apDevices);

    // Populate routing tables.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void
HandoverExperiment::SetupApplications()
{
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);

    // Install UDP Echo Servers on both AP nodes.
    ApplicationContainer serverApp1 = echoServer.Install(m_wifiApNodes.Get(0));
    ApplicationContainer serverApp2 = echoServer.Install(m_wifiApNodes.Get(1));
    serverApp1.Start(Seconds(0.0));
    serverApp1.Stop(Seconds(m_simulationTime));
    serverApp2.Start(Seconds(0.0));
    serverApp2.Stop(Seconds(m_simulationTime));

    // Configure UDP Echo Clients for the pedestrian nodes.
    // Group A uses AP1; Group B uses AP2.
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
    // Set fixed colors for AP nodes.
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(0)->GetId(), 0, 0, 255);   // Dark blue for AP1.
    m_anim->UpdateNodeColor(m_wifiApNodes.Get(1)->GetId(), 255, 0, 0);   // Dark red for AP2.

    // Enable PCAP generation for each AP device and each station device.
    YansWifiPhyHelper wifiPhyHelper;
    // Iterate over AP devices.
    for (uint32_t i = 0; i < m_apDevices.GetN(); ++i)
    {
        std::string fileName = "data/handover/ap_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_apDevices.Get(i), true,
                                 YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
    // Iterate over station devices.
    for (uint32_t i = 0; i < m_staDevices.GetN(); ++i)
    {
        std::string fileName = "data/handover/sta_" + std::to_string(i) + ".pcap";
        wifiPhyHelper.EnablePcap(fileName, m_staDevices.Get(i), true,
                                 YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    }
}

void
HandoverExperiment::RestoreBeaconIntervalForAP1()
{
    NS_LOG_INFO("Restoring AP1 beacon transmissions.");
    if (m_ap1Mac)
    {
        m_ap1Mac->SetBeaconInterval(Seconds(0.1024));
    }
    m_handoverTriggeredAP1 = false;
}

void
HandoverExperiment::RestoreBeaconIntervalForAP2()
{
    NS_LOG_INFO("Restoring AP2 beacon transmissions.");
    if (m_ap2Mac)
    {
        m_ap2Mac->SetBeaconInterval(Seconds(0.1024));
    }
    m_handoverTriggeredAP2 = false;
}

void
HandoverExperiment::CheckRssiAndTriggerHandover()
{
    // Retrieve positions of AP nodes.
    Vector ap1Pos = m_wifiApNodes.Get(0)->GetObject<MobilityModel>()->GetPosition();
    Vector ap2Pos = m_wifiApNodes.Get(1)->GetObject<MobilityModel>()->GetPosition();

    // Check for Group A: initially associated with AP1.
    uint32_t triggerCountA = 0;
    for (uint32_t i = 0; i < m_groupA.GetN(); ++i)
    {
        Ptr<Node> node = m_groupA.Get(i);
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        double rssiAp1 = EstimateRssi(pos, ap1Pos);
        double rssiAp2 = EstimateRssi(pos, ap2Pos);
        if (rssiAp2 > (rssiAp1 + m_handoverMargin))
        {
            triggerCountA++;
        }
    }
    if (triggerCountA > (m_groupA.GetN() / 2) && !m_handoverTriggeredAP1)
    {
        NS_LOG_INFO("Handover Triggered for Group A: RSSI favors AP2 over AP1.");
        // Set a long beacon interval (≈5 s) to simulate handover by disabling AP1 beacons.
        m_ap1Mac->SetBeaconInterval(Seconds(4883 * 0.001024));
        m_handoverTriggeredAP1 = true;
        Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreBeaconIntervalForAP1, this);
    }

    // Check for Group B: initially associated with AP2.
    uint32_t triggerCountB = 0;
    for (uint32_t i = 0; i < m_groupB.GetN(); ++i)
    {
        Ptr<Node> node = m_groupB.Get(i);
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        double rssiAp2 = EstimateRssi(pos, ap2Pos);
        double rssiAp1 = EstimateRssi(pos, ap1Pos);
        if (rssiAp1 > (rssiAp2 + m_handoverMargin))
        {
            triggerCountB++;
        }
    }
    if (triggerCountB > (m_groupB.GetN() / 2) && !m_handoverTriggeredAP2)
    {
        NS_LOG_INFO("Handover Triggered for Group B: RSSI favors AP1 over AP2.");
        m_ap2Mac->SetBeaconInterval(Seconds(4883 * 0.001024));
        m_handoverTriggeredAP2 = true;
        Simulator::Schedule(Seconds(5.0), &HandoverExperiment::RestoreBeaconIntervalForAP2, this);
    }

    // Reschedule this function every second until the simulation ends.
    if (Simulator::Now().GetSeconds() < m_simulationTime)
    {
        Simulator::Schedule(Seconds(1.0), &HandoverExperiment::CheckRssiAndTriggerHandover, this);
    }
}
