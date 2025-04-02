//
// Created by Jakub Dubec on 01/04/2025.
//

#ifndef MONADCOUNT_SIM_TERMINALFACTORY_HPP
#define MONADCOUNT_SIM_TERMINALFACTORY_HPP

#include "ns3/application.h"

using namespace ns3;

class TerminalFactory {
public:
    virtual ~TerminalFactory() {}
    // Install a terminal on the provided node.
    virtual void InstallTerminal(ns3::Ptr<ns3::Node> node) = 0;
};

#endif //MONADCOUNT_SIM_TERMINALFACTORY_HPP
