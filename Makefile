# Project directories
PROJECT_ROOT := $(shell pwd)
NS3_SRC_DIR := $(PROJECT_ROOT)/extern/ns3-src
NS3_DIST_DIR := $(PROJECT_ROOT)/extern/ns3-dist
BUILD_DIR := $(PROJECT_ROOT)/build
BIN_DIR := $(PROJECT_ROOT)/bin

# Default target
all: ns3 build

# Build NS-3
ns3:
	@echo "Building NS-3..."
	cd $(NS3_SRC_DIR) && \
	./ns3 configure --enable-examples --enable-tests --enable-mpi --prefix $(NS3_DIST_DIR) && \
	./ns3 build && \
	./ns3 install

# Configure and build the project
build:
	@echo "Building MonadCountSim..."
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Clean everything including NS-3 build
clean-all: clean
	cd $(NS3_SRC_DIR) && ./ns3 clean
	rm -rf $(NS3_DIST_DIR)

.PHONY: all ns3 build clean clean-all