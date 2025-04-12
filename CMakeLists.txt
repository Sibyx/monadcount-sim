cmake_minimum_required(VERSION 3.14)
project(monadcount-sim LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =======================================================================
# External Dependencies (Using Git Submodules)
# =======================================================================

# Include nlohmann::json from the submodule (instead of using FetchContent)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/nlohmann_json)

# ns-3 configuration (using the pre-built ns-3 install from your submodule)
set(NS3_CMAKE_DIR "${CMAKE_SOURCE_DIR}/extern/ns3-dist/lib/cmake/ns3")
find_package(ns3 REQUIRED
        PATHS ${NS3_CMAKE_DIR}
        NO_DEFAULT_PATH  # Avoid using system-wide ns-3 installations
)

add_subdirectory(${CMAKE_SOURCE_DIR}/extern/netanim)

# =======================================================================
# Project Subdirectories
# =======================================================================
# Add your own modules/subdirectories
add_subdirectory(src/core)
add_subdirectory(src/experiments)
add_subdirectory(src/factories)

# =======================================================================
# Create Executable and Link Libraries
# =======================================================================
add_executable(monadcount_sim src/main.cpp)

target_link_libraries(monadcount_sim
        PRIVATE
        # Standard libraries:
        nlohmann_json::nlohmann_json

        # ns-3 libraries:
        ns3::core
        ns3::network
        ns3::internet
        ns3::mobility
        ns3::applications
        ns3::wifi
        ns3::netanim

        # Simulation libraries:
        monadcount_sim::factories_pedestrians
        monadcount_sim::core
        monadcount_sim_experiments
)

# =======================================================================
# Runtime Output & Custom Targets
# =======================================================================
set_target_properties(monadcount_sim PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_target(run
        COMMAND ${CMAKE_BINARY_DIR}/bin/monadcount-sim
        DEPENDS monadcount-sim
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running simulation..."
)
