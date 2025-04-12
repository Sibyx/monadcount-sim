#ifndef MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
#define MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP


#include "monadcount_sim/core/Scenario.hpp"

class DoorToDoorExperiment : public monadcount_sim::core::Scenario {
public:
    DoorToDoorExperiment();
    ~DoorToDoorExperiment() override = default;

protected:
    void Run(monadcount_sim::core::ScenarioEnvironment &env) override;

private:
    double m_simulationTime;
};

#endif //MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP