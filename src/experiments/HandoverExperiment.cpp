// HandoverExperiment.cpp
#include "HandoverExperiment.hpp"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverExperiment");

HandoverExperiment::HandoverExperiment ()
        : m_numPedestrians(10),
          m_simulationTime(60.0),
          m_roomLength(50.0),
          m_roomWidth(30.0)
{
}

void HandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    NS_LOG_INFO("Setting up Handover Experiment...");

    // --------------------------------------------------
    // 1) Create Nodes
    // --------------------------------------------------
    // Create two AP nodes
    NodeContainer wifiApNodes;
    wifiApNodes.Create(2);

    // Create station nodes (pedestrians)
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(m_numPedestrians);

    // --------------------------------------------------
    // 2) Set up a Single Wi-Fi Channel for both APs & Stations
    // --------------------------------------------------
    // For re-association (handover) to be possible, both APs must be on the same channel.
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    // --------------------------------------------------
    // 3) Configure Wi-Fi
    // --------------------------------------------------
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    // Both APs use the same SSID, e.g., "eduroam".
    Ssid ssid = Ssid("eduroam");

    // 3a) AP configuration (both APs)
    WifiMacHelper macAp;
    macAp.SetType("ns3::ApWifiMac",
                  "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(wifiPhy, macAp, wifiApNodes);

    // 3b) Station configuration
    WifiMacHelper macSta;
    macSta.SetType("ns3::StaWifiMac",
                   "Ssid", SsidValue(ssid),
                   "ActiveProbing", BooleanValue(true));
    NetDeviceContainer staDevices = wifi.Install(wifiPhy, macSta, wifiStaNodes);

    // --------------------------------------------------
    // 4) Mobility
    // --------------------------------------------------
    // (a) AP Mobility: Place the two APs on the same plane with different initial positions.
    MobilityHelper mobilityAp;
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(wifiApNodes);

    // Position AP1 on the left side and AP2 on the right side.
    Ptr<MobilityModel> apMob1 = wifiApNodes.Get(0)->GetObject<MobilityModel>();
    apMob1->SetPosition(Vector(10.0, m_roomWidth / 2.0, 2.0));
    Ptr<MobilityModel> apMob2 = wifiApNodes.Get(1)->GetObject<MobilityModel>();
    apMob2->SetPosition(Vector(40.0, m_roomWidth / 2.0, 2.0));

    // (b) Stations Mobility: Distribute stations across the entire room and update their movement speed.
    MobilityHelper mobilitySta;
    mobilitySta.SetPositionAllocator(
            "ns3::RandomRectanglePositionAllocator",
            "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomLength) + "]"),
            "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomWidth) + "]")
    );
    mobilitySta.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                 "Bounds", RectangleValue(Rectangle(0, m_roomLength, 0, m_roomWidth)),
                                 "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=3.0]"));
    mobilitySta.Install(wifiStaNodes);

    // --------------------------------------------------
    // 5) Internet Stack + IP Addressing
    // --------------------------------------------------
    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterfaces = address.Assign(apDevices);
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // --------------------------------------------------
    // 6) Applications
    // --------------------------------------------------
    // For illustration, set up a UDP Echo application.
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);
    // Install the echo server on AP2 (which will eventually be preferred)
    ApplicationContainer serverApp = echoServer.Install(wifiApNodes.Get(1));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(m_simulationTime));

    UdpEchoClientHelper echoClient(apInterfaces.GetAddress(1), echoPort);
    echoClient.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i)
    {
        clientApps.Add(echoClient.Install(wifiStaNodes.Get(i)));
    }
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(m_simulationTime));

    // --------------------------------------------------
    // 7) Handover Trigger Event
    // --------------------------------------------------
    // At 30 seconds, move AP1 to a location where its signal degrades relative to AP2.
    // This should cause some stations (which are roaming across the room)
    // to detect the stronger beacon from AP2 and trigger a re-association.
    Simulator::Schedule(Seconds(30.0), [=, this](){
        NS_LOG_INFO("Handover Event: Moving AP1 to a new location (0.0, mid room).");
        Ptr<MobilityModel> mob = wifiApNodes.Get(0)->GetObject<MobilityModel>();
        mob->SetPosition(Vector(0.0, m_roomWidth / 2.0, 2.0));
        // This repositioning degrades AP1's signal for many nodes,
        // allowing the stations to find a stronger signal from AP2.
    });

    // --------------------------------------------------
    // 8) Tracing and Animation
    // --------------------------------------------------
    // Enable PCAP tracing for analysis.
    wifiPhy.EnablePcap("data/handover/ap", apDevices);
    AnimationInterface anim("data/handover/netanim.xml");
    anim.SetMaxPktsPerTraceFile(500000);

    // --------------------------------------------------
    // 9) Run Simulation
    // --------------------------------------------------
    Simulator::Stop(Seconds(m_simulationTime));
    NS_LOG_INFO("Running Handover Simulation...");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Handover Simulation complete.");
}
