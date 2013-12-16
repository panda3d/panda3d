# We require 2.8.4 because earlier releases had a bug
# with invalid HEADER_FILE_ONLY behavior in MSVC 2010.
cmake_minimum_required(VERSION 2.8.4)
project(Panda3D)

# Set Panda's CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# This is where dtool_config.h gets generated.
include_directories("${CMAKE_BINARY_DIR}")

# Configure Panda3D
include(dtool/PandaMacros.cmake)
include(dtool/PandaVersion.cmake)
include(dtool/Packages.cmake)
include(dtool/Config.cmake)

# Include Panda3D packages
add_subdirectory(dtool)
add_subdirectory(panda)
