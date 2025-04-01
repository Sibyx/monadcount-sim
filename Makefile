# Root-level Makefile for monadcount-sim
# This Makefile builds ns-3 and then compiles the simulation code,
# storing all binaries in the build/ directory. It also provides a target
# to run the compiled simulation.

# Compiler and flags for simulation code
CC = g++
CFLAGS=-std=c++20 -Isrc/include -Iextern/ns-3/build/include -Iextern/nlohmann_json/single_include
# Link against the required ns-3 libraries. Adjust the list as needed.
LDFLAGS=-Lextern/ns-3/build/lib -lns3_core -lns3_network -lns3_internet -lns3_mobility -lns3_applications

# Directories and target definitions
BUILD_DIR = build
SRC = $(wildcard src/*.cc)
# Object files will be stored in build/; we replace "src/" with "$(BUILD_DIR)/" in the filenames.
OBJ = $(patsubst src/%.cc, $(BUILD_DIR)/%.o, $(SRC))
TARGET = $(BUILD_DIR)/monadcount-sim

# Default target: build ns-3 first, then the simulation executable
all: ns3 $(TARGET)

# Build ns-3 by invoking ns3 inside the extern/ns-3 directory.
ns3:
	@echo "Building ns-3..."
	@cd extern/ns-3 && ./ns3 configure --enable-examples --enable-tests && ./ns3 build
	@echo "ns-3 build complete."

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the simulation executable from object files.
$(TARGET): $(BUILD_DIR) $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Pattern rule: compile each source file into the build directory.
$(BUILD_DIR)/%.o: src/%.cc | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Target to run the compiled simulation.
run: $(TARGET)
	@echo "Running monadcount-sim..."
	./$(TARGET)

# Clean both simulation objects and ns-3 build artifacts.
clean:
	@echo "Cleaning simulation build..."
	rm -rf extern/ns3-dist
	@echo "Cleaning ns-3 build..."
	@cd extern/ns3-src && ./ns3 clean

.PHONY: all ns3 clean run
