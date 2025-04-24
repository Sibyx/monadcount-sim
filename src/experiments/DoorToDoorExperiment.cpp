#include "DoorToDoorExperiment.hpp"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"

#include <vector>
#include <random>
#include <cmath>
#include <sys/stat.h>
#include <limits>

NS_LOG_COMPONENT_DEFINE("DoorToDoorExperiment");

void
DoorToDoorExperiment::LogEvent(uint32_t id, const std::string& what)
{
    NS_LOG_INFO("Time=" << ns3::Simulator::Now().GetSeconds()
                        << "s: Pedestrian " << id << " " << what);
}

DoorToDoorExperiment::DoorToDoorExperiment()
        : m_simulationTime(60.0),
          m_numPedestrians(50),
          m_roomLength(50.0),
          m_roomWidth(30.0)
{
}


void
DoorToDoorExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    NS_LOG_INFO("Running Experiment: Door-to-Door Wi-Fi");

    //
    // 1) Gather “door” spawn positions
    //
    std::vector<Vector> doorPos;
    if (!env.doors.empty()) {
        for (auto &d : env.doors) {
            doorPos.emplace_back(d.x, d.y, 1.5);
            NS_LOG_INFO("  door at (" << d.x << "," << d.y << ")");
        }
    } else {
        NS_LOG_WARN("No doors in env.doors; using 4 mid-wall defaults");
        doorPos = {
                {0.0,             m_roomWidth/2, 1.5},
                {m_roomLength,    m_roomWidth/2, 1.5},
                {m_roomLength/2,  0.0,           1.5},
                {m_roomLength/2,  m_roomWidth,   1.5}
        };
    }
    const uint32_t nDoors = doorPos.size();

    //
    // 2) Gather “terminal” AP positions
    //
    std::vector<Vector> apPos;
    if (env.terminalNodes.GetN() > 0) {
        for (uint32_t i = 0; i < env.terminalNodes.GetN(); ++i) {
            Ptr<Node> termNode = env.terminalNodes.Get(i);
            Ptr<MobilityModel> mm = termNode->GetObject<MobilityModel>();
            Vector p = mm->GetPosition();
            apPos.emplace_back(p.x, p.y, 2.0);
        }
    } else {
        NS_LOG_WARN("No env.terminalNodes; using 4 corner APs");
        apPos = {
                {0.0,           0.0,           2.0},
                {m_roomLength,  0.0,           2.0},
                {0.0,           m_roomWidth,   2.0},
                {m_roomLength,  m_roomWidth,   2.0}
        };
    }
    const uint32_t nAps = apPos.size();

    //
    // 3) Create nodes
    //
    NodeContainer apNodes;   apNodes.Create(nAps);
    NodeContainer pedNodes;  pedNodes.Create(m_numPedestrians);

    //
    // 4) Mobility
    //
    // 4a) APs fixed
    MobilityHelper apMob;
    apMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    apMob.Install(apNodes);
    for (uint32_t i = 0; i < nAps; ++i) {
        apNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(apPos[i]);
    }

    // 4b) Pedestrians via WaypointMobilityModel
    MobilityHelper pedMob;
    pedMob.SetMobilityModel("ns3::WaypointMobilityModel");
    pedMob.Install(pedNodes);

    //
    // 5) Wi-Fi channel & PHY
    //
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
    Ptr<YansWifiChannel> wifiChannel = channel.Create();

    YansWifiPhyHelper phy;
    phy.SetErrorRateModel("ns3::NistErrorRateModel");
    phy.SetChannel(wifiChannel);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    //
    // 6) Install AP devices
    //
    Ssid ssid = Ssid("door-net");
    WifiMacHelper macAp;
    macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevs = wifi.Install(phy, macAp, apNodes);

    //
    // 7) Install STA devices on pedestrians
    //
    WifiMacHelper macSta;
    macSta.SetType("ns3::StaWifiMac",
                   "Ssid", SsidValue(ssid),
                   "ActiveProbing", BooleanValue(true));
    NetDeviceContainer staDevs = wifi.Install(phy, macSta, pedNodes);

    //
    // 8) Internet stack + IP
    //
    InternetStackHelper stack;
    stack.Install(apNodes);
    stack.Install(pedNodes);

    Ipv4AddressHelper addr;
    addr.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apIfs  = addr.Assign(apDevs);
    Ipv4InterfaceContainer staIfs = addr.Assign(staDevs);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // collect AP IPs
    std::vector<Ipv4Address> apAddrs(nAps);
    for (uint32_t i = 0; i < nAps; ++i) {
        apAddrs[i] = apIfs.GetAddress(i);
    }

    //
    // 9) Enable PCAP tracing
    //
    ::mkdir("data",            0755);
    ::mkdir("data/doortodoor", 0755);
    phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("data/doortodoor/ap",  apDevs);
    phy.EnablePcap("data/doortodoor/sta", staDevs);

    //
    // 10) Random variables
    //
    auto makeRv = [&](double lo, double hi) {
        Ptr<UniformRandomVariable> rv = CreateObject<UniformRandomVariable>();
        rv->SetAttribute("Min", DoubleValue(lo));
        rv->SetAttribute("Max", DoubleValue(hi));
        return rv;
    };
    Ptr<UniformRandomVariable> doorRv      = makeRv(0,       nDoors - 1);
    Ptr<UniformRandomVariable> speedRv     = makeRv(0.8,     1.4);
    Ptr<UniformRandomVariable> roamCountRv = makeRv(1,       4);
    Ptr<UniformRandomVariable> roamXrv     = makeRv(0.0,     m_roomLength);
    Ptr<UniformRandomVariable> roamYrv     = makeRv(0.0,     m_roomWidth);
    Ptr<UniformRandomVariable> dwellStartRv= makeRv(0.3*m_simulationTime,
                                                    0.6*m_simulationTime);
    Ptr<UniformRandomVariable> dwellLenRv  = makeRv(2.0,     6.0);

    const uint16_t leafPort = 9000;

    //
    // 11) Plan each pedestrian’s waypoints & delivery
    //
    for (uint32_t i = 0; i < pedNodes.GetN(); ++i) {
        Ptr<Node> ped = pedNodes.Get(i);
        Ptr<WaypointMobilityModel> wmm =
                ped->GetObject<WaypointMobilityModel>();
        double now = 0.0;

        // a) spawn at random door
        uint32_t startDoor = doorRv->GetInteger();
        Vector curr = doorPos[startDoor];
        wmm->AddWaypoint(Waypoint(Seconds(now), curr));

        // b) roam random points until dwell start
        double tDwellStart = dwellStartRv->GetValue();
        int roamCount = (int) roamCountRv->GetValue();
        for (int r = 0; r < roamCount; ++r) {
            Vector dest(roamXrv->GetValue(),
                        roamYrv->GetValue(),
                        1.5);
            double dist = std::hypot(dest.x - curr.x,
                                     dest.y - curr.y);
            double dt = dist / speedRv->GetValue();
            if (now + dt > tDwellStart) break;
            now += dt;
            wmm->AddWaypoint(Waypoint(Seconds(now), dest));
            curr = dest;
        }

        // c) move to terminal location & deliver
        Vector terminal(roamXrv->GetValue(),
                        roamYrv->GetValue(),
                        1.5);
        {
            double dist = std::hypot(terminal.x - curr.x,
                                     terminal.y - curr.y);
            double dt = dist / speedRv->GetValue();
            now += dt;
            if (now > tDwellStart) now = tDwellStart;
            wmm->AddWaypoint(Waypoint(Seconds(now), terminal));

            // pick nearest AP
            uint32_t bestAp = 0;
            double bestD = std::numeric_limits<double>::max();
            for (uint32_t a = 0; a < nAps; ++a) {
                double d = std::hypot(terminal.x - apPos[a].x,
                                      terminal.y - apPos[a].y);
                if (d < bestD) { bestD = d; bestAp = a; }
            }

            // send “leaflet” burst
            OnOffHelper onoff("ns3::UdpSocketFactory",
                              InetSocketAddress(apAddrs[bestAp], leafPort));
            onoff.SetAttribute("DataRate",  StringValue("500kbps"));
            onoff.SetAttribute("PacketSize", UintegerValue(256));
            double tDwellLen = dwellLenRv->GetValue();
            ApplicationContainer burst = onoff.Install(ped);
            burst.Start(Seconds(now));
            burst.Stop(Seconds(std::min(now + tDwellLen,
                                        m_simulationTime)));

            // log delivery
            Simulator::Schedule(Seconds(now),
                                &DoorToDoorExperiment::LogEvent,
                                i,
                                std::string("delivering to AP#")
                                + std::to_string(bestAp));
        }

        // d) dwell
        now = std::min(now + dwellLenRv->GetValue(), m_simulationTime);
        wmm->AddWaypoint(Waypoint(Seconds(now), terminal));

        // e) exit via nearest door
        double bestD2 = std::numeric_limits<double>::max();
        Vector exitDoor = doorPos[0];
        for (auto &dp : doorPos) {
            double d = std::hypot(terminal.x - dp.x,
                                  terminal.y - dp.y);
            if (d < bestD2) { bestD2 = d; exitDoor = dp; }
        }
        if (now < m_simulationTime) {
            double dt = bestD2 / speedRv->GetValue();
            double tExit = std::min(now + dt, m_simulationTime);
            wmm->AddWaypoint(Waypoint(Seconds(tExit), exitDoor));
            Simulator::Schedule(Seconds(tExit),
                                &DoorToDoorExperiment::LogEvent,
                                i,
                                std::string("exited via door"));
            if (tExit < m_simulationTime) {
                wmm->AddWaypoint(
                        Waypoint(Seconds(m_simulationTime), exitDoor)
                );
            }
        }
    }

    //
    // 12) Run
    //
    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Door-to-Door Wi-Fi experiment complete.");
}