cmake_minimum_required(VERSION 2.8)
project(Panda3D)

## Set Panda's CMake Module path ##
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

## This is where dtool_config.h gets generated ##
include_directories("${CMAKE_BINARY_DIR}")

## Configure Panda3D ##(
include(dtool/PandaVersion.cmake)
include(dtool/Packages.cmake)
include(dtool/Config.cmake)

## Include Panda3D Packages ##
add_subdirectory(dtool)
add_subdirectory(panda)
