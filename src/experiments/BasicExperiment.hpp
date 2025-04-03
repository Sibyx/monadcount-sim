// src/experiments/BasicExperiment.hpp
#ifndef MONADCOUNT_SIM_BASICEXPERIMENT_HPP
#define MONADCOUNT_SIM_BASICEXPERIMENT_HPP

#include "Scenario.hpp"

class BasicExperiment : public Scenario {
public:
    BasicExperiment();
    ~BasicExperiment() override = default;

protected:
    void Run(ScenarioEnvironment& env) override;

private:
    uint32_t m_numPedestrians;
    double m_simulationTime;
    double m_roomLength;
    double m_roomWidth;
};

#endif //MONADCOUNT_SIM_BASICEXPERIMENT_HPP