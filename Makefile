# Project directories
PROJECT_ROOT := $(shell pwd)
NS3_SRC_DIR := $(PROJECT_ROOT)/extern/ns3-src
NS3_DIST_DIR := $(PROJECT_ROOT)/extern/ns3-dist
NETANIM_DIR := $(PROJECT_ROOT)/extern/netanim
NETSIMULYZER_DIR := $(PROJECT_ROOT)/extern/netsimulyzer-ns3
NETSIMULYZER_GUI_DIR := $(PROJECT_ROOT)/extern/netsimulyzer-gui
NETSIMULYZER_GUI_BUILD := $(NETSIMULYZER_GUI_DIR)/build
NETSIMULYZER_GUI_BIN := $(NETSIMULYZER_GUI_BUILD)/netsimulyzer
BUILD_DIR := $(PROJECT_ROOT)/build
BIN_DIR := $(PROJECT_ROOT)/bin

# Default target
all: ns3 build netsimulyzer-gui

# Build NS-3
ns3:
	@echo "Building NS-3..."
	rm -f $(NS3_SRC_DIR)/contrib/netsimulyzer
	ln -s $(NETSIMULYZER_DIR) $(NS3_SRC_DIR)/contrib/netsimulyzer
	cd $(NS3_SRC_DIR) && \
	./ns3 configure --enable-tests --prefix $(NS3_DIST_DIR) && \
	./ns3 build && \
	./ns3 install

netsimulyzer-gui:
	@echo "Building NetSimulyzer GUI..."
	mkdir -p $(NETSIMULYZER_GUI_BUILD)
		cd $(NETSIMULYZER_GUI_BUILD) && \
    		cmake .. \
    			-DCMAKE_PREFIX_PATH="$(shell brew --prefix minizip)" \
    			-DCMAKE_EXE_LINKER_FLAGS="-L$(shell brew --prefix minizip)/lib" && \
    		make -j
	@echo "Copying NetSimulyzer binary to $(BIN_DIR)..."
	mkdir -p $(BIN_DIR)
	cp $(NETSIMULYZER_GUI_BIN) $(BUILD_DIR)/bin/netsimulyzer


# Configure and build the project
build:
	@echo "Building MonadCountSim..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)

# Clean everything including NS-3 build
clean-all: clean
	cd $(NS3_SRC_DIR) && ./ns3 clean
	rm -rf $(NS3_DIST_DIR)
	rm -rf $(NETSIMULYZER_GUI_BUILD)

.PHONY: all ns3 netanim build clean clean-all