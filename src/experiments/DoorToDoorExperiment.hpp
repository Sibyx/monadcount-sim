#ifndef MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP
#define MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP

#include "Scenario.hpp"  // Updated path to match project structure

class DoorToDoorExperiment : public Scenario {
public:
    DoorToDoorExperiment();
    ~DoorToDoorExperiment() override = default;

protected:
    void Run(ScenarioEnvironment &env) override;

private:
    double m_simulationTime;
};

#endif //MONADCOUNT_SIM_DOORTODOOREXPERIMENT_HPP