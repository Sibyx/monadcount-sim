#include "BasicExperiment.hpp"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

// If you have a "ScenarioEnvironment.hpp" or other headers, include them here.
// #include "ScenarioEnvironment.hpp"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PedestrianWifiSim");

BasicExperiment::BasicExperiment()
        : m_numPedestrians(10),
          m_simulationTime(60.0),
          m_roomLength(50.0),
          m_roomWidth(30.0)
{
}

void BasicExperiment::Run(ScenarioEnvironment &env)
{
    // --------------------------------------------------
    // 1) Create Nodes
    // --------------------------------------------------
    NodeContainer wifiApNodes;
    wifiApNodes.Create(2); // two APs

    // First set of station nodes (half)
    NodeContainer wifiStaNodes1;
    uint32_t half = m_numPedestrians / 2;
    wifiStaNodes1.Create(half);

    // Second set of station nodes (the rest)
    NodeContainer wifiStaNodes2;
    wifiStaNodes2.Create(m_numPedestrians - half);

    // --------------------------------------------------
    // 2) Separate Wi-Fi Channels
    // --------------------------------------------------
    // AP #1 + its stations on channel1/phy1
    YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy1;
    phy1.SetChannel(channel1.Create());

    // AP #2 + its stations on channel2/phy2
    // (Could vary propagation parameters or frequency if desired)
    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy2;
    phy2.SetChannel(channel2.Create());

    // --------------------------------------------------
    // 3) Configure Wi-Fi
    // --------------------------------------------------
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    // We'll still use the same SSID for simplicity—
    // but note that with physically separate channels, stations won't roam between them in practice.
    Ssid ssid = Ssid("eduroam");

    // 3a) AP #1
    WifiMacHelper macAp1;
    macAp1.SetType("ns3::ApWifiMac",
                   "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice1 = wifi.Install(phy1, macAp1, wifiApNodes.Get(0));

    // 3b) AP #2
    WifiMacHelper macAp2;
    macAp2.SetType("ns3::ApWifiMac",
                   "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice2 = wifi.Install(phy2, macAp2, wifiApNodes.Get(1));

    // 3c) Stations for AP #1
    WifiMacHelper macSta1;
    macSta1.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid),
                    "ActiveProbing", BooleanValue(true));
    NetDeviceContainer staDevices1 = wifi.Install(phy1, macSta1, wifiStaNodes1);

    // 3d) Stations for AP #2
    WifiMacHelper macSta2;
    macSta2.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid),
                    "ActiveProbing", BooleanValue(true));
    NetDeviceContainer staDevices2 = wifi.Install(phy2, macSta2, wifiStaNodes2);

    // --------------------------------------------------
    // 4) Mobility
    // --------------------------------------------------
    // (a) APs - stationary
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(wifiApNodes);

    // Position AP #1 near the center
    Ptr<MobilityModel> apMob1 = wifiApNodes.Get(0)->GetObject<MobilityModel>();
    apMob1->SetPosition(Vector(m_roomLength / 2, m_roomWidth / 2, 2.0));

    // Position AP #2 a bit to the right
    Ptr<MobilityModel> apMob2 = wifiApNodes.Get(1)->GetObject<MobilityModel>();
    apMob2->SetPosition(Vector((m_roomLength / 2) + 20.0, m_roomWidth / 2, 2.0));

    // (b) Stations #1
    MobilityHelper mobilitySta1;
    mobilitySta1.SetPositionAllocator(
            "ns3::RandomRectanglePositionAllocator",
            "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomLength) + "]"),
            "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomWidth) + "]")
    );
    mobilitySta1.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(0, m_roomLength, 0, m_roomWidth)),
                                  "Speed", StringValue("ns3::UniformRandomVariable[Min=0.5|Max=1.5]"));
    mobilitySta1.Install(wifiStaNodes1);

    // (c) Stations #2
    MobilityHelper mobilitySta2;
    mobilitySta2.SetPositionAllocator(
            "ns3::RandomRectanglePositionAllocator",
            "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomLength) + "]"),
            "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomWidth) + "]")
    );
    mobilitySta2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(0, m_roomLength, 0, m_roomWidth)),
                                  "Speed", StringValue("ns3::UniformRandomVariable[Min=0.5|Max=1.5]"));
    mobilitySta2.Install(wifiStaNodes2);

    // --------------------------------------------------
    // 5) Internet Stack + Multiple Subnets
    // --------------------------------------------------
    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    stack.Install(wifiStaNodes1);
    stack.Install(wifiStaNodes2);

    // Subnet 1: 10.1.1.x for AP #1 + stations #1
    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ap1Interfaces = address1.Assign(apDevice1);
    Ipv4InterfaceContainer sta1Interfaces = address1.Assign(staDevices1);

    // Subnet 2: 10.1.2.x for AP #2 + stations #2
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ap2Interfaces = address2.Assign(apDevice2);
    Ipv4InterfaceContainer sta2Interfaces = address2.Assign(staDevices2);

    // Populate routing (for cross-subnet traffic)
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // --------------------------------------------------
    // 6) Applications
    // --------------------------------------------------
    // 6a) UDP Echo on AP #1
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);
    ApplicationContainer serverApp = echoServer.Install(wifiApNodes.Get(0)); // AP #1
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(m_simulationTime));

    // Echo clients from stations #1 to AP #1
    UdpEchoClientHelper echoClient(ap1Interfaces.GetAddress(0), echoPort);
    echoClient.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < wifiStaNodes1.GetN(); ++i)
    {
        clientApps.Add(echoClient.Install(wifiStaNodes1.Get(i)));
    }
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(m_simulationTime));

    // 6b) OnOff UDP on AP #2
    // We'll send traffic from stations #2 to AP #2
    uint16_t onOffPort = 9000;
    OnOffHelper onOffUdp("ns3::UdpSocketFactory",
                         InetSocketAddress(ap2Interfaces.GetAddress(0), onOffPort));
    onOffUdp.SetAttribute("DataRate", StringValue("2Mbps"));
    onOffUdp.SetAttribute("PacketSize", UintegerValue(512));
    onOffUdp.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffUdp.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer onOffApps;
    for (uint32_t i = 0; i < wifiStaNodes2.GetN(); ++i)
    {
        auto app = onOffUdp.Install(wifiStaNodes2.Get(i));
        app.Start(Seconds(2.0 + 0.2 * i));
        app.Stop(Seconds(m_simulationTime));
        onOffApps.Add(app);
    }

    // A sink on AP #2 to receive OnOff traffic
    PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
                             InetSocketAddress(Ipv4Address::GetAny(), onOffPort));
    ApplicationContainer sinkApp = sinkUdp.Install(wifiApNodes.Get(1)); // AP #2
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(m_simulationTime));

    // --------------------------------------------------
    // 7) Tracing (PCAP)
    // --------------------------------------------------
    // We'll enable pcap on the device(s) for each AP
    phy1.EnablePcap("pedestrian_wifi_ap1", apDevice1);
    phy2.EnablePcap("pedestrian_wifi_ap2", apDevice2);

    // --------------------------------------------------
    // 8) Run
    // --------------------------------------------------
    Simulator::Stop(Seconds(m_simulationTime));
    NS_LOG_INFO("Running Simulation...");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation complete.");
}
