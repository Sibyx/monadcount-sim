#ifndef MONADCOUNT_SIM_BASICEXPERIMENT_HPP
#define MONADCOUNT_SIM_BASICEXPERIMENT_HPP

#include "monadcount_sim/core/Scenario.hpp"
#include <string>

class BasicExperiment : public monadcount_sim::core::Scenario {
public:
    BasicExperiment();
    ~BasicExperiment() override = default;

    void SetPropagationModel(const std::string& model);

protected:
    void Run(monadcount_sim::core::ScenarioEnvironment& env) override;

private:
    uint32_t m_numPedestrians;
    double   m_simulationTime;
    double   m_roomLength;
    double   m_roomWidth;

    std::string m_propagationModel;
};

#endif // MONADCOUNT_SIM_BASICEXPERIMENT_HPP
