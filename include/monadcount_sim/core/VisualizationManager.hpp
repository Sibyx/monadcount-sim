//
// Created by Jakub Dubec on 13/04/2025.
//

#ifndef MONADCOUNT_SIM_VISUALIZATIONMANAGER_HPP
#define MONADCOUNT_SIM_VISUALIZATIONMANAGER_HPP

#include <ns3/animation-interface.h>
#include <ns3/netsimulyzer-module.h>
#include <ns3/node-container.h>
#include <ns3/ptr.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <functional>

namespace monadcount_sim::core {

    using namespace ns3;

    class VisualizationManager {
    public:
        struct GroupVisualConfig {
            std::string labelPrefix;
            std::function<std::tuple<double, double, double>(uint32_t nodeId, int apId)> netsimColorFunc;
            std::function<std::tuple<uint8_t, uint8_t, uint8_t>(uint32_t nodeId, int apId)> netanimColorFunc;
            bool enableTrail = false;
            std::string nodeModel = "sphere"; // NetSimulyzer model
        };

        VisualizationManager();
        ~VisualizationManager();

        void EnableNetAnim(const std::string& filename);
        void EnableNetSimulyzer(const std::string& filename);

        void RegisterGroup(const std::string& groupId,
                           const ns3::NodeContainer& nodes,
                           const GroupVisualConfig& config);

        void Initialize();

        void OnNodeAssociated(uint32_t nodeId, int apId);
        void OnHandoverEvent(uint32_t nodeId, int fromAp, int toAp, double time);
        void LogMessage(const std::string& message);

    private:
        ns3::AnimationInterface* m_anim = nullptr;

        ns3::Ptr<ns3::netsimulyzer::Orchestrator> m_orchestrator;
        ns3::Ptr<ns3::netsimulyzer::LogStream> m_logStream;
        std::unique_ptr<ns3::netsimulyzer::NodeConfigurationHelper> m_nodeHelper;

        struct Group {
            ns3::NodeContainer nodes;
            GroupVisualConfig config;
        };

        std::map<std::string, Group> m_groups;
        std::map<uint32_t, std::string> m_nodeToGroup;
    };

} // namespace monadcount_sim::core

#endif // MONADCOUNT_SIM_VISUALIZATIONMANAGER_HPP
