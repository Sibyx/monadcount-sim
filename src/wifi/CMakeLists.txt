add_library(monadcount_sim_wifi
        RssiBasedAssocManager.cpp
)

target_include_directories(monadcount_sim_wifi PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(monadcount_sim_wifi
        PUBLIC
        ns3::core
        ns3::network
        ns3::applications
        ns3::wifi
)

add_library(monadcount_sim::wifi ALIAS monadcount_sim_wifi)