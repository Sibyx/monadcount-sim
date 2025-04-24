#include "monadcount_sim/core/VisualizationManager.hpp"
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/netsimulyzer-module.h>
#include <sstream>

namespace monadcount_sim::core {

    NS_LOG_COMPONENT_DEFINE("VisualizationManager");

    using namespace ns3;

    VisualizationManager::VisualizationManager() = default;

    VisualizationManager::~VisualizationManager() {
        delete m_anim;
    }

    void VisualizationManager::EnableNetAnim(const std::string& filename) {
        m_anim = new AnimationInterface(filename);
    }

    void VisualizationManager::EnableNetSimulyzer(const std::string& filename) {
        m_orchestrator = CreateObject<netsimulyzer::Orchestrator>(filename);
        m_logStream = CreateObject<netsimulyzer::LogStream>(m_orchestrator);
        m_nodeHelper = std::make_unique<netsimulyzer::NodeConfigurationHelper>(m_orchestrator);
    }

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
        if (m_nodeHelper) {
            for (const auto& [groupId, group] : m_groups) {
                const auto& config = group.config;
                m_nodeHelper->Set("Model", StringValue(config.nodeModel));
                m_nodeHelper->Set("EnableMotionTrail", BooleanValue(config.enableTrail));
                m_nodeHelper->Install(group.nodes);
            }
        }

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
    }

    void VisualizationManager::OnNodeAssociated(uint32_t nodeId, int apId) {
        auto it = m_nodeToGroup.find(nodeId);
        if (it == m_nodeToGroup.end()) return;

        const auto& groupId = it->second;
        const auto& config = m_groups.at(groupId).config;

        if (m_anim) {
            auto [r, g, b] = config.netanimColorFunc(nodeId, apId);
            m_anim->UpdateNodeColor(nodeId, r, g, b);
        }

        if (m_nodeHelper) {
            auto [rD, gD, bD] = config.netsimColorFunc(nodeId, apId);
            auto node = NodeList::GetNode(nodeId);
            auto nodeCfg = node->GetObject<netsimulyzer::NodeConfiguration>();
            if (nodeCfg) {
                nodeCfg->SetBaseColor(netsimulyzer::Color3(rD, gD, bD));
            }
        }
    }

    void VisualizationManager::OnHandoverEvent(uint32_t nodeId, int fromAp, int toAp, double time) {
        if (m_logStream) {
            std::ostringstream os;
            os << "[" << time << "s] Node " << nodeId << " handed over: AP" << fromAp << " -> AP" << toAp;
            *m_logStream << os.str() << "\n";
        }
    }

    void VisualizationManager::LogMessage(const std::string& message) {
        if (m_logStream) {
            *m_logStream << "[" << Simulator::Now().GetSeconds() << "s] " << message << "\n";
        }
    }

} // namespace monadcount_sim::core
