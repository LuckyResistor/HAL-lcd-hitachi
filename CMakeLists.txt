# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Set the project name
project(HAL-lcd-hitachi)

# Make sure we use the C++17 compiler standard
set(CMAKE_CXX_STANDARD 17)

# Create a static library.
add_library(HAL-lcd-hitachi AfBackConnection.hpp HConnection.hpp HDisplay.cpp HDisplay.hpp HMCPConnection.hpp)

