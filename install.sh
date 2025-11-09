#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

# go to the directory of the script to avoid accidental build in wrong dir
cd "$(dirname "$0")"

# Create build directory
mkdir -p build
cd build

# Run CMake configuration
cmake .. || { echo "CMake configuration failed"; exit 1; }

# Build the project
cmake --build . || { echo "Build failed"; exit 1; }

# Install the module
sudo cmake --install . || { echo "Install failed"; exit 1; }

echo "Build and installation completed successfully."