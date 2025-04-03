cmake_minimum_required(VERSION 3.14)
project(monadcount-sim LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set variables for ns-3 directories (adjust if needed)
set(NS3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/ns3-dist/include)
set(NS3_LIB_DIR ${CMAKE_SOURCE_DIR}/extern/ns3-dist/lib)

# =======================================================================
# Include directories for your project and dependencies.
# =======================================================================
# Project headers.
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/models)
include_directories(${CMAKE_SOURCE_DIR}/src/experiments)

# ns-3 includes (both built and source)
include_directories(${NS3_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/extern/ns-3/src)

# nlohmann_json header-only include.
include_directories(${CMAKE_SOURCE_DIR}/extern/nlohmann_json/single_include)

# =======================================================================
# Gather source files from src/ directory.
# =======================================================================
file(GLOB SRC_FILES "src/**/*.cpp" "src/main.cpp")

# Create the simulation executable.
add_executable(monadcount-sim ${SRC_FILES})

# Specify output directory for the binary.
set_target_properties(monadcount-sim PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =======================================================================
# Link ns-3 libraries.
# =======================================================================
# Add the ns-3 library directory.
link_directories(${NS3_LIB_DIR})

#find_package(ns3 REQUIRED PATHS /home/frank/Installs/ns3/cmake-cache)
find_package(ns3 REQUIRED PATHS ${CMAKE_SOURCE_DIR}/extern/ns3-dist/lib/cmake)


# Link against the required ns-3 modules. (Adjust the list as needed.)
target_link_libraries(
        monadcount-sim
        ns3::core
        ns3::network
        ns3::internet
        ns3::mobility
        ns3::applications
        ns3::wifi
)

# =======================================================================
# Add a custom target to run the simulation.
# =======================================================================
#add_custom_target(run
#        COMMAND ${CMAKE_BINARY_DIR}/bin/monadcount-sim
#        DEPENDS monadcount-sim
#        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#        COMMENT "Running monadcount-sim..."
#)
