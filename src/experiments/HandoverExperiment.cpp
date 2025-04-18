#include "HandoverExperiment.hpp"

#ifdef WITH_NETANIM
#include <ns3/animation-interface.h>
#endif

#include "ns3/ipv4-global-routing-helper.h"
#include <cmath>
#include <string>
#include <map>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HandoverExperiment");

// Tweak parameters as needed
HandoverExperiment::HandoverExperiment ()
        : m_numAp(5),
          m_numPedestrians(25),
          m_simulationTime(60.0),
          m_roomLength(50.0),
          m_roomWidth(30.0),
          m_handoverMargin(5.0),
          m_txPower_dBm(20.0),
          m_pathLossExponent(3.0),
          // Mobility model: ConstantVelocity|RandomWalk2d|GaussMarkov|Waypoint
          m_mobilityModel("GaussMarkov"),
          m_anim(nullptr)
{
}

void
HandoverExperiment::Run(monadcount_sim::core::ScenarioEnvironment &env)
{
    /*
    CommandLine cmd;
    cmd.AddValue("nAp",      "Number of APs",          m_numAp);
    cmd.AddValue("nPed",     "Total pedestrians",      m_numPedestrians);
    cmd.AddValue("mobModel", "Mobility model: ConstantVelocity|RandomWalk2d|GaussMarkov|Waypoint",
                 m_mobilityModel);
    cmd.Parse();
    */
    NS_LOG_INFO("Setting up RSSI-based Handover Experiment...");
    SetupNodes();
    SetupWifi();
    SetupMobility();
    SetupInternet();
    SetupApplications();
    SetupTracing();

    // Schedule handover checks
    Simulator::Schedule(Seconds(1.0),
                        &HandoverExperiment::CheckRssiAndTriggerHandover,
                        this);

    // At end, dump summary
    Simulator::Schedule(Seconds(m_simulationTime + 0.1), [this]() {
        NS_LOG_UNCOND("=== Handover Summary per Node ===");
        for (auto &kv : m_handoverCount) {
            NS_LOG_UNCOND(" Node " << kv.first
                                   << " performed " << kv.second
                                   << " handovers");
        }
    });

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
    double dist = std::sqrt(dx*dx + dy*dy);
    if (dist < 1.0) dist = 1.0;
    return m_txPower_dBm - 10.0 * m_pathLossExponent * std::log10(dist);
}

void
HandoverExperiment::SetupNodes()
{
    // Create APs
    m_wifiApNodes.Create(m_numAp);

    // Divide pedestrians evenly into groups
    uint32_t perGroup = m_numPedestrians / m_numAp;
    uint32_t rem      = m_numPedestrians % m_numAp;
    m_groups.resize(m_numAp);

    for (uint32_t i = 0; i < m_numAp; ++i) {
        uint32_t cnt = perGroup + (i == m_numAp-1 ? rem : 0);
        m_groups[i].Create(cnt);
        for (uint32_t j = 0; j < cnt; ++j) {
            uint32_t id = m_groups[i].Get(j)->GetId();
            m_nodeAssociation[id] = i;
            m_nodeTriggered [id] = false;
            m_handoverCount [id] = 0;
        }
    }
}

void
HandoverExperiment::SetupWifi()
{
    // Channel, PHY, MAC
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
    YansWifiPhyHelper phy; phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    Ssid ssid("eduroam");
    // APs
    WifiMacHelper macAp;
    macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    m_apDevices = wifi.Install(phy, macAp, m_wifiApNodes);

    // STAs
    WifiMacHelper macSta;
    macSta.SetType("ns3::StaWifiMac",
                   "Ssid", SsidValue(ssid),
                   "ActiveProbing", BooleanValue(true));
    for (auto &grp : m_groups) {
        NetDeviceContainer d = wifi.Install(phy, macSta, grp);
        m_staDevices.Add(d);
    }
}

void
HandoverExperiment::SetupMobility()
{
    // 1) Place APs along the x‑axis, centered in y:
    MobilityHelper mobAp;
    mobAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobAp.Install(m_wifiApNodes);
    for (uint32_t i = 0; i < m_numAp; ++i) {
        double x = 5.0 + i * (m_roomLength - 10.0) / (m_numAp - 1);
        m_wifiApNodes.Get(i)
                ->GetObject<MobilityModel>()
                ->SetPosition(Vector(x, m_roomWidth/2.0, 2.0));
    }

    // 2) Pedestrian groups
    for (uint32_t i = 0; i < m_numAp; ++i) {
        // each group starts in a 10‑m band on their AP side:
        double xmin = (i == 0 ? 0.0 : m_roomLength - 10.0);
        double xmax = (i == 0 ? 10.0 : m_roomLength);

        MobilityHelper mobPed;
        mobPed.SetPositionAllocator(
                "ns3::RandomRectanglePositionAllocator",
                "X", StringValue("ns3::UniformRandomVariable[Min=" +
                                 std::to_string(xmin) + "|Max=" +
                                 std::to_string(xmax) + "]"),
                "Y", StringValue("ns3::UniformRandomVariable[Min=5.0|Max=25.0]"));

        if (m_mobilityModel == "RandomWalk2d") {
            // bias walk ±10° around due‑east or due‑west
            double dirCenter = (i == 0 ? 0.0 : M_PI);
            double delta    = M_PI / 18.0;
            mobPed.SetMobilityModel(
                    "ns3::RandomWalk2dMobilityModel",
                    "Mode", StringValue("Time"),
                    "Time", StringValue("2s"),
                    "Speed", StringValue(
                            "ns3::UniformRandomVariable[Min=0.5|Max=1.5]"),
                    "Bounds", RectangleValue(
                            Rectangle(0, m_roomLength, 0, m_roomWidth)),
                    "Direction", StringValue(
                            "ns3::UniformRandomVariable[Min=" +
                            std::to_string(dirCenter - delta) +
                            "|Max=" +
                            std::to_string(dirCenter + delta) + "]"));
        }
        else if (m_mobilityModel == "GaussMarkov") {
            mobPed.SetMobilityModel(
                    "ns3::GaussMarkovMobilityModel",
                    // how often the model updates
                    "TimeStep",        TimeValue(Seconds(0.5)),
                    // drift control (0 → pure randomness; 1 → pure drift)
                    "Alpha",           DoubleValue(0.85),
                    // constant speed of ±1 m/s
                    "MeanVelocity",    StringValue(
                            "ns3::UniformRandomVariable[Min=" +
                            std::to_string(i==0?+1.0:-1.0) +
                            "|Max=" +
                            std::to_string(i==0?+1.0:-1.0) + "]"),
                    // small Gaussian speed fluctuation around that mean
                    "NormalVelocity",  StringValue(
                            "ns3::NormalRandomVariable[Mean=0.0|Variance=0.1|Bound=0.5]"),
                    // constant direction (0 = east, π = west)
                    "MeanDirection",   StringValue(
                            "ns3::ConstantRandomVariable[Constant=" +
                            std::to_string(i==0?0.0:M_PI) + "]"),
                    // small angular jitter
                    "NormalDirection", StringValue(
                            "ns3::NormalRandomVariable[Mean=0.0|Variance=0.05|Bound=0.2]")
            );
        }

        else if (m_mobilityModel == "Waypoint") {
            mobPed.SetMobilityModel("ns3::WaypointMobilityModel");
        }
        else {
            // default: constant velocity toward the other AP
            mobPed.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
        }

        // install mobility on this group
        mobPed.Install(m_groups[i]);

        // 3) post‑install: seed velocities or waypoints
        if (m_mobilityModel == "ConstantVelocity") {
            auto urv = CreateObject<UniformRandomVariable>();
            urv->SetAttribute("Min", DoubleValue(-0.5));
            urv->SetAttribute("Max", DoubleValue( 0.5));
            double vx = (i == 0 ? +1.0 : -1.0);
            for (uint32_t j = 0; j < m_groups[i].GetN(); ++j) {
                Ptr<ConstantVelocityMobilityModel> cvm =
                        m_groups[i].Get(j)->GetObject<ConstantVelocityMobilityModel>();
                cvm->SetVelocity(Vector(vx, urv->GetValue(), 0.0));
            }
        }
        else if (m_mobilityModel == "Waypoint") {
            auto urv = CreateObject<UniformRandomVariable>();
            urv->SetAttribute("Min", DoubleValue(5.0));
            urv->SetAttribute("Max", DoubleValue(m_roomWidth - 5.0));
            for (uint32_t j = 0; j < m_groups[i].GetN(); ++j) {
                Ptr<Node> node = m_groups[i].Get(j);
                Ptr<WaypointMobilityModel> wpm =
                        node->GetObject<WaypointMobilityModel>();
                // start position at t=0
                Vector p0 = node->GetObject<MobilityModel>()->GetPosition();
                wpm->AddWaypoint(Waypoint(Seconds(0.0), p0));
                // 25% time midpoint on opposite wall
                Vector p1(
                        (i == 0 ? m_roomLength - 5.0 : 5.0),
                        urv->GetValue(), 2.0);
                wpm->AddWaypoint(Waypoint(Seconds(m_simulationTime*0.25), p1));
                // 75% time final waypoint
                Vector p2(
                        (i == 0 ? m_roomLength - 5.0 : 5.0),
                        urv->GetValue(), 2.0);
                wpm->AddWaypoint(Waypoint(Seconds(m_simulationTime*0.75), p2));
            }
        }
    }
}

void
HandoverExperiment::SetupInternet()
{
    InternetStackHelper stack;
    stack.Install(m_wifiApNodes);
    for (auto &grp : m_groups) stack.Install(grp);

    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    addr.Assign(m_apDevices);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void
HandoverExperiment::SetupApplications()
{
    uint16_t port = 7;
    UdpEchoServerHelper server(port);
    for (uint32_t i = 0; i < m_numAp; ++i) {
        auto srv = server.Install(m_wifiApNodes.Get(i));
        srv.Start(Seconds(0.0));
        srv.Stop(Seconds(m_simulationTime));
    }

    for (uint32_t i = 0; i < m_numAp; ++i) {
        // look up AP’s IP
        Ipv4Address apAddr = m_wifiApNodes.Get(i)
                ->GetObject<Ipv4>()
                ->GetAddress(1,0)
                .GetLocal();
        UdpEchoClientHelper client(apAddr, port);
        client.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
        client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        client.SetAttribute("PacketSize", UintegerValue(1024));

        auto apps = client.Install(m_groups[i]);
        apps.Start(Seconds(1.0));
        apps.Stop(Seconds(m_simulationTime));
    }
}

void
HandoverExperiment::SetupTracing()
{
#ifdef WITH_NETANIM
    m_anim = new AnimationInterface("data/handover/netanim.xml");
    // color APs dark-blue / dark-red
    for (uint32_t i = 0; i < m_numAp; ++i) {
      uint8_t r = (i==0?0:255), g = 0, b = (i==0?255:0);
      m_anim->UpdateNodeColor(
        m_wifiApNodes.Get(i)->GetId(), r, g, b);
    }
#endif

    YansWifiPhyHelper phy;
    for (uint32_t i = 0; i < m_apDevices.GetN(); ++i) {
        phy.EnablePcap("data/handover/hand_ap"+std::to_string(i),
                       m_apDevices.Get(i), true);
    }
    for (uint32_t i = 0; i < m_staDevices.GetN(); ++i) {
        phy.EnablePcap("data/handover/hand_sta"+std::to_string(i),
                       m_staDevices.Get(i), true);
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
    // get AP positions
    std::vector<Vector> apPos(m_numAp);
    for (uint32_t i = 0; i < m_numAp; ++i) {
        apPos[i] = m_wifiApNodes.Get(i)
                ->GetObject<MobilityModel>()
                ->GetPosition();
    }

    // for each pedestrian
    for (auto &grp : m_groups) {
        for (uint32_t j = 0; j < grp.GetN(); ++j) {
            Ptr<Node> node = grp.Get(j);
            auto pos = node->GetObject<MobilityModel>()->GetPosition();
            uint32_t id = node->GetId();

            // compute RSSIs
            std::vector<double> rssis(m_numAp);
            for (uint32_t i = 0; i < m_numAp; ++i) {
                rssis[i] = EstimateRssi(pos, apPos[i]);
            }

            int cur = m_nodeAssociation[id], best = cur;
            for (int i = 0; i < (int)m_numAp; ++i) {
                if (rssis[i] > rssis[best] + m_handoverMargin) {
                    best = i;
                }
            }

            if (best != cur && !m_nodeTriggered[id]) {
                NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s: Node " << id
                << " handover AP" << cur
                << "→AP" << best);
                m_nodeAssociation[id] = best;
                m_nodeTriggered [id] = true;
                m_handoverCount [id]++;
#ifdef WITH_NETANIM
                // light‐color the new AP color
                uint8_t r = (best==0?150:255),
                        g = 150,
                        b = (best==0?255:150);
                m_anim->UpdateNodeColor(id, r, g, b);
#endif
                Simulator::Schedule(Seconds(5.0),
                                    &HandoverExperiment::RestoreNodeTriggered,
                                    this, id);
            }
        }
    }

    if (Simulator::Now().GetSeconds() < m_simulationTime) {
        Simulator::Schedule(Seconds(1.0),
                            &HandoverExperiment::CheckRssiAndTriggerHandover,
                            this);
    }
}
