//
// Created by Jakub Dubec on 13/04/2025.
//

#ifndef MONADCOUNT_SIM_GAUSSMARKOVHANDOVEREXPERIMENT_HPP
#define MONADCOUNT_SIM_GAUSSMARKOVHANDOVEREXPERIMENT_HPP

#include "HandoverExperiment.hpp"

class GaussMarkovHandoverExperiment : public HandoverExperiment
{
public:
    GaussMarkovHandoverExperiment();

    /**
     * Runs the simulation with Gauss-Markov pedestrian mobility.
     */
    void Run(monadcount_sim::core::ScenarioEnvironment &env) override;

protected:
    /**
     * Overrides the base SetupMobility() to use GaussMarkovMobilityModel.
     */
    void SetupMobility();
    void SetupTracing();
    void SetupVisualization();
};

#endif //MONADCOUNT_SIM_GAUSSMARKOVHANDOVEREXPERIMENT_HPP
