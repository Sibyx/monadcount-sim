add_library(monadcount_sim_core
        GeoJsonParser.cpp
        Scenario.cpp
        ScenarioEnvironmentBuilder.cpp
        ScenarioFactory.cpp
        VisualizationManager.cpp
)

target_link_libraries(monadcount_sim_core
        PUBLIC
        ns3::core
        ns3::network
        nlohmann_json::nlohmann_json
)

target_include_directories(monadcount_sim_core PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

add_library(monadcount_sim::core ALIAS monadcount_sim_core)