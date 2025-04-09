#include "BasicExperiment.hpp"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

// #include "ScenarioEnvironment.hpp" // If you need your own scenario/environment headers

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
    // Create nodes: a set of station nodes + one AP node
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(m_numPedestrians);

    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    // Create channel + PHY
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    // Configure Wi-Fi
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    // Create Wi-Fi MAC for stations
    WifiMacHelper mac;
    Ssid ssid = Ssid("eduroam");

    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(true));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, wifiStaNodes);

    // Create Wi-Fi MAC for AP
    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifi.Install(phy, mac, wifiApNode);

    // Mobility setup
    MobilityHelper mobility;
    mobility.SetPositionAllocator(
            "ns3::RandomRectanglePositionAllocator",
            "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomLength) + "]"),
            "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(m_roomWidth) + "]")
    );

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0, m_roomLength, 0, m_roomWidth)),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=0.5|Max=1.5]"));
    mobility.Install(wifiStaNodes);

    // AP is stationary in the center
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);
    Ptr<MobilityModel> apMobility = wifiApNode.Get(0)->GetObject<MobilityModel>();
    apMobility->SetPosition(Vector(m_roomLength / 2, m_roomWidth / 2, 2));

    // Install IP stack
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apInterfaces = address.Assign(apDevice);

    // Use UdpEchoServerHelper instead of UdpServerHelper
    uint16_t port = 7; // port 7 is traditionally labeled "echo" by Wireshark
    UdpEchoServerHelper echoServer(port);

    // Install the echo server on the AP
    ApplicationContainer serverApp = echoServer.Install(wifiApNode.Get(0));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(m_simulationTime));

    // Use UdpEchoClientHelper instead of UdpClientHelper
    UdpEchoClientHelper echoClient(apInterfaces.GetAddress(0), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(4294967295u));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // Install echo clients on each station node
    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i)
    {
        clientApps.Add(echoClient.Install(wifiStaNodes.Get(i)));
    }
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(m_simulationTime));

    // Enable pcap tracing on the AP device
    phy.EnablePcap("pedestrian_wifi", apDevice);

    // Run the simulation
    Simulator::Stop(Seconds(m_simulationTime));
    NS_LOG_INFO("Running Simulation...");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Simulation complete.");
}
