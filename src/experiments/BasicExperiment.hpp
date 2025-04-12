#ifndef MONADCOUNT_SIM_BASICEXPERIMENT_HPP
#define MONADCOUNT_SIM_BASICEXPERIMENT_HPP


#include "monadcount_sim/core/Scenario.hpp"

class BasicExperiment : public monadcount_sim::core::Scenario {
public:
    BasicExperiment();
    ~BasicExperiment() override = default;

protected:
    void Run(monadcount_sim::core::ScenarioEnvironment& env) override;

private:
    uint32_t m_numPedestrians;
    double m_simulationTime;
    double m_roomLength;
    double m_roomWidth;
};

#endif //MONADCOUNT_SIM_BASICEXPERIMENT_HPP