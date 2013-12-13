cmake_minimum_required(VERSION 2.8)
project(Panda3D)

## Panda Version Information ##
set(PANDA_VERSION 1 9 0)
set(PANDA_OFFICIAL_VERSION FALSE CACHE BOOL "If true, signifies this is a tagged release rather than a repository snapshot. Scripts that generate source tarballs and/or binary releases for distribution should explicitly set this to true.")
set(PANDA_DISTRIBUTOR "homebuilt" CACHE STRING "This should be set by whoever provides a particular distribution of Panda. If you build your own Panda, leave this unchanged.")

set(P3D_PLUGIN_VERSION 1 0 4)
set(P3D_COREAPI_VERSION ${P3D_PLUGIN_VERSION} 1)


## Set CMake Find<Foo> Module path ##
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

## This is where dtool_config.h gets generated ##
include_directories("${CMAKE_BINARY_DIR}/include")

## Include Panda3D Packages ##
add_subdirectory(dtool)
add_subdirectory(panda)
