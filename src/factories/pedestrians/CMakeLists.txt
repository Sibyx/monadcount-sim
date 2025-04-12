add_library(monadcount_sim_factories_pedestrians STATIC
        RandomWalkDoorPedestrianFactory.cpp
)

target_include_directories(monadcount_sim_factories_pedestrians PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(monadcount_sim_factories_pedestrians
        PUBLIC
        ns3::core
        ns3::network
)

add_library(monadcount_sim::factories_pedestrians ALIAS monadcount_sim_factories_pedestrians)
