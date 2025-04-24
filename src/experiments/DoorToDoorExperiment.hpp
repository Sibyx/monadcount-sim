#ifndef MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
#define MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP

#include "monadcount_sim/core/Scenario.hpp"
#include <cstdint>
#include <string>

namespace monadcount_sim {
    namespace core { class ScenarioEnvironment; }
}

class DoorToDoorExperiment : public monadcount_sim::core::Scenario {
public:
    DoorToDoorExperiment();
    ~DoorToDoorExperiment() override = default;

    /// How many pedestrians to spawn (default 50)
    void SetNumPedestrians(uint32_t n) { m_numPedestrians = n; }

protected:
    void Run(monadcount_sim::core::ScenarioEnvironment &env) override;

private:
    double   m_simulationTime;
    uint32_t m_numPedestrians;

    double   m_roomLength;
    double   m_roomWidth;

    /// Internal logger
    static void LogEvent(uint32_t pedId, const std::string &what);
};

#endif // MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
