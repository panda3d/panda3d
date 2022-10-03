cmake_minimum_required(VERSION 2.8)
project(squish)

# By default, enable SSE2 instructions on x64.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  if(APPLE AND CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    set(DEFAULT_SSE OFF)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(DEFAULT_SSE OFF)
  else()
    set(DEFAULT_SSE ON)
  endif()
else()
  set(DEFAULT_SSE OFF)
endif()

option(USE_ALTIVEC "Define to 1 to use Altivec instructions" OFF)
option(USE_SSE "Define to 1 to use SSE2 instructions" ${DEFAULT_SSE})
option(BUILD_SHARED_LIBS "Define to build libsquish as shared library" OFF)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX d CACHE STRING "Suffix to add to the library name in debug builds")

  if(USE_SSE)
    add_definitions(/arch:SSE2 /DSQUISH_USE_SSE=2)
  endif()
else()
  if(USE_ALTIVEC)
    add_definitions(-maltivec -DSQUISH_USE_ALTIVEC=1)
  endif()
  if(USE_SSE)
    add_definitions(-msse -msse2 -DSQUISH_USE_SSE=2)
  endif()

  # Easiest way to solve a missing include of limits.h.
  add_definitions(-DINT_MAX=2147483647)
endif()

set(SRC alpha.cpp clusterfit.cpp colourblock.cpp colourfit.cpp colourset.cpp maths.cpp rangefit.cpp singlecolourfit.cpp squish.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(squish ${SRC})

install(FILES squish.h DESTINATION include)
install(TARGETS squish DESTINATION lib)

