add_library(monadcount_sim_experiments
        BasicExperiment.cpp
        DoorToDoorExperiment.cpp
        HandoverExperiment.cpp

)

target_include_directories(monadcount_sim_experiments PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(monadcount_sim_experiments
        PUBLIC
        ns3::core
        ns3::network
        ns3::internet
        ns3::mobility
        ns3::applications
        ns3::wifi
)
