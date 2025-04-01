# MonadCountSim: Indoor GeoJSON-based NS-3 environment

**Work in progress**

The codebase sucks bigly. Will improve soon. Me wanna PhD.

## Install

```shell
git submodule add https://gitlab.com/nsnam/ns-3-dev.git extern/ns3-src
git submodule add https://github.com/nlohmann/json.git extern/nlohmann_json

cd extern/ns3-src && git checkout tags/ns-3.43
cd extern/nlohmann_json && git checkout tags/v3.11.3
```

## Toolchain Setup

### Using ExternalProject

```cmake
include(ExternalProject)

# =======================================================================
# Build ns-3 as an external project using its own build system (waf).
# =======================================================================
ExternalProject_Add(ns3
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/ns-3
        CONFIGURE_COMMAND ./ns3 configure --enable-examples --enable-tests --enable-mpi
        BUILD_COMMAND ./ns3 build
        INSTALL_COMMAND ""
        # Build in source because ns-3 uses its own directory structure.
        BUILD_IN_SOURCE 1
)
```



```shell
# Sync submodules
git submodule update --init --recursive      

# Configure and build NS3 
cd extern/ns3-src
# Ugly, need to take notes. Me lazy fuck
./ns3 configure --enable-examples --enable-tests --enable-mpi --prefix /Users/jdubec/Projects/MonadCount/monadcount-sim/extern/ns3-dist
./ns3 build
./ns3 install

# Execute cmake: TBD
```

```mermaid
flowchart TD
    A[GeoJSON File] --> B[GeoJSONParser]
    B --> C((List of Feature Objects))

    C --> D[Ns3EnvironmentBuilder]
    D --> E((Ns3Environment))

%% Environment internal collections
    E --> E1[AP Nodes]
    E --> E2[Obstacles]
    E --> E3[Seats]
    E --> E4[Doors]
    E --> E5[Terminals]
    E --> E6[EventDispatcher]

%% Dynamic Actors
    E4 --> F[Door]
    E3 --> G[Seat]
    E5 --> H[Terminal]
    F --> I[PedestrianFactory]
    I --> J[Pedestrian]

%% Event flow
    J -- "hitSeat" --> G
    J -- "foundDoor" --> F
    H -- "taskEvent" --> J
    F -- "spawnRequest" --> I
    I -- "pedestrianSpawned" --> J
    E6 -- "control events" --> F
    E6 -- "global events" --> J

%% Scenario Manager
    K[Scenario Manager]
    K --> I
    K -->|Monitors events| E6
    K -->|Controls spawn rate| F

    style A fill:#ffd,stroke:#333,stroke-width:1px
    style B fill:#ccf,stroke:#333,stroke-width:1px
    style C fill:#afa,stroke:#333,stroke-width:1px
    style D fill:#ccf,stroke:#333,stroke-width:1px
    style E fill:#afa,stroke:#333,stroke-width:1px
    style F fill:#fcc,stroke:#333,stroke-width:1px
    style G fill:#fcc,stroke:#333,stroke-width:1px
    style H fill:#fcc,stroke:#333,stroke-width:1px
    style I fill:#cfc,stroke:#333,stroke-width:1px
    style J fill:#cfc,stroke:#333,stroke-width:1px
    style K fill:#ccf,stroke:#333,stroke-width:1px

```