#include "monadcount_sim/core/VisualizationManager.hpp"
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#ifdef WITH_NETSIMULYZER
#include <ns3/netsimulyzer-module.h>
#endif
#include <sstream>

namespace monadcount_sim::core {

    NS_LOG_COMPONENT_DEFINE("VisualizationManager");

    using namespace ns3;

    VisualizationManager::VisualizationManager() = default;

    VisualizationManager::~VisualizationManager() {
#ifdef WITH_NETANIM
        delete m_anim;
#endif
    }

    // Enable NetAnim visualization if requested
#ifdef WITH_NETANIM
    void VisualizationManager::EnableNetAnim(const std::string& filename) {
        m_anim = new AnimationInterface(filename);
    }
#endif

    // Enable NetSimulyzer visualization if requested
#ifdef WITH_NETSIMULYZER
    void VisualizationManager::EnableNetSimulyzer(const std::string& filename) {
        m_orchestrator = CreateObject<netsimulyzer::Orchestrator>(filename);
        m_logStream = CreateObject<netsimulyzer::LogStream>(m_orchestrator);
        m_nodeHelper = std::make_unique<netsimulyzer::NodeConfigurationHelper>(m_orchestrator);
    }
#endif

    void VisualizationManager::RegisterGroup(const std::string& groupId,
                                             const NodeContainer& nodes,
                                             const GroupVisualConfig& config) {
        Group group;
        group.nodes = nodes;
        group.config = config;
        m_groups[groupId] = group;

        for (uint32_t i = 0; i < nodes.GetN(); ++i) {
            Ptr<Node> node = nodes.Get(i);
            m_nodeToGroup[node->GetId()] = groupId;
        }
    }

    void VisualizationManager::Initialize() {
#ifdef WITH_NETSIMULYZER
        // Initialize NetSimulyzer groups
        if (m_nodeHelper) {
            for (const auto& [groupId, group] : m_groups) {
                const auto& config = group.config;
                m_nodeHelper->Set("Model", StringValue(config.nodeModel));
                m_nodeHelper->Set("EnableMotionTrail", BooleanValue(config.enableTrail));
                m_nodeHelper->Install(group.nodes);
            }
        }
#endif
#ifdef WITH_NETANIM
        // Initialize NetAnim groups
        if (m_anim) {
            for (const auto& [groupId, group] : m_groups) {
                const auto& config = group.config;
                for (uint32_t i = 0; i < group.nodes.GetN(); ++i) {
                    uint32_t nodeId = group.nodes.Get(i)->GetId();
                    auto [r, g, b] = config.netanimColorFunc(nodeId, 0);
                    m_anim->UpdateNodeColor(nodeId, r, g, b);
                }
            }
        }
#endif
    }

    void VisualizationManager::OnNodeAssociated(uint32_t nodeId, int apId) {
        auto it = m_nodeToGroup.find(nodeId);
        if (it == m_nodeToGroup.end()) return;

        const auto& groupId = it->second;
        const auto& config = m_groups.at(groupId).config;

// Update NetAnim visualization if enabled
#ifdef WITH_NETANIM
        if (m_anim) {
            auto [r, g, b] = config.netanimColorFunc(nodeId, apId);
            m_anim->UpdateNodeColor(nodeId, r, g, b);
        }
#endif

// Update NetSimulyzer visualization if enabled
#ifdef WITH_NETSIMULYZER
        if (m_nodeHelper) {
            auto [rD, gD, bD] = config.netsimColorFunc(nodeId, apId);
            auto node = NodeList::GetNode(nodeId);
            auto nodeCfg = node->GetObject<netsimulyzer::NodeConfiguration>();
            if (nodeCfg) {
                nodeCfg->SetBaseColor(netsimulyzer::Color3(rD, gD, bD));
            }
        }
#endif
    }

    void VisualizationManager::OnHandoverEvent(uint32_t nodeId, int fromAp, int toAp, double time) {
// Log handover event for NetSimulyzer if enabled
#ifdef WITH_NETSIMULYZER
        if (m_logStream) {
            std::ostringstream os;
            os << "[" << time << "s] Node " << nodeId << " handed over: AP" << fromAp << " -> AP" << toAp;
            *m_logStream << os.str() << "\n";
        }
#endif
    }

    void VisualizationManager::LogMessage(const std::string& message) {
// Log custom message for NetSimulyzer if enabled
#ifdef WITH_NETSIMULYZER
    if (m_logStream) {
        *m_logStream << "[" << Simulator::Now().GetSeconds() << "s] " << message << "\n";
    }
#endif
    }

} // namespace monadcount_sim::core
