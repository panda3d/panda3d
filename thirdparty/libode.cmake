# Made by rdb, based on the premake4.lua that's in the build directory of the
# ode source tree.  Released in the public domain.  Use as you will.

cmake_minimum_required(VERSION 2.8)
project(ode)

######################################################################
## Configuration options
######################################################################

#option(WITH_DEMOS "Builds the demo applications and DrawStuff library" OFF)
#option(WITH_TESTS "Builds the unit test application" OFF)
option(WITH_GIMPACT "Use GIMPACT for trimesh collisions (experimental)" OFF)
option(WITH_LIBCCD "Uses libccd for handling some collision tests absent in ODE." ON)
option(NO_DIF "Exclude DIF (Dynamics Interchange Format) exports" OFF)
option(NO_TRIMESH "Exclude trimesh collision geometry" OFF)
option(WITH_OU "Use TLS for global caches (allows threaded collision checks for separated spaces)" OFF)
option(WITH_BUILTIN_THREADING_IMPL "Include built-in multithreaded threading implementation (still must be created and assigned to be used)" OFF)
option(NO_THREADING_INTF "Disable threading interface support (external implementations may not be assigned; overrides WITH_BUILTIN_THREADING_IMPL)" OFF)
option(16BIT_INDICES "Use 16-bit indices for trimeshes (default is 32-bit)" OFF)
option(OLD_TRIMESH "Use old OPCODE trimesh-trimesh collider" OFF)
option(BUILD_SHARED_LIBS "Build shared (DLL) version of the library" OFF)

if(MSVC)
  # In MSVC, we can build both configurations.
  option(ONLY_SINGLE "Only use single-precision math" OFF)
  option(ONLY_DOUBLE "Only use double-precision math" OFF)
else()
  # In other systems, we must choose at configure time.
  option(WITH_DOUBLE_PRECISION "Use double-precision math" OFF)
  if(WITH_DOUBLE_PRECISION)
    set(ONLY_DOUBLE ON)
    set(ONLY_SINGLE OFF)
  else()
    set(ONLY_DOUBLE OFF)
    set(ONLY_SINGLE ON)
  endif()
endif()

include_directories(include ode/src)

if(WIN32)
  add_definitions(-DWIN32)
endif()

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
  add_definitions(/D_CRT_SECURE_NO_DEPRECATE /D_USE_MATH_DEFINES /wd4244)
endif()

if(APPLE)
  add_definitions(-DMAC_OS_X_VERSION=1050)
endif()

######################################################################
## Write a custom <config.h> to build, based on the supplied flags
######################################################################

if(NO_TRIMESH)
elseif(WITH_GIMPACT)
  set(dTRIMESH_ENABLED 1)
  set(dTRIMESH_GIMPACT 1)
else()
  set(dTRIMESH_ENABLED 1)
  set(dTRIMESH_OPCODE 1)
endif()

if(WITH_OU OR NOT NO_THREADING_INTF)
  set(dOU_ENABLED 1)
  set(dATOMICS_ENABLED 1)
endif()

if(WITH_OU)
  set(dTLS_ENABLED 1)
endif()

if(NO_THREADING_INTF)
  set(dTHREADING_INTF_DISABLED 1)
elseif(WITH_BUILDIN_THREADING_IMPL)
  set(dBUILTIN_THREADING_IMPL_ENABLED 1)
endif()

if(16BIT_INDICES)
  set(dTRIMESH_16BIT_INDICES 1)
endif()

if(OLD_TRIMESH)
  set(dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER 1)
endif()

configure_file(ode/src/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/ode/src/config.h)

############################
## Write precision headers
############################

if(ONLY_SINGLE)
  set(ODE_PRECISION dSINGLE)
  set(CCD_PRECISION CCD_SINGLE)
elseif(ONLY_DOUBLE)
  set(ODE_PRECISION dDOUBLE)
  set(CCD_PRECISION CCD_DOUBLE)
else()
  set(ODE_PRECISION dUNDEFINEDPRECISION)
  set(CCD_PRECISION CCD_UNDEFINEDPRECISION)
endif()

configure_file(include/ode/precision.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/ode/precision.h)
configure_file(libccd/src/ccd/precision.h.in ${CMAKE_CURRENT_SOURCE_DIR}/libccd/src/ccd/precision.h)

######################################################################
## The ODE library project
######################################################################

include_directories(
  ode/src/joints
  OPCODE
  GIMPACT/include
  libccd/src
)

file(GLOB ODE_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/ode/*.h
  ode/src/joints/*.h
  ode/src/joints/*.cpp
  ode/src/*.h
  ode/src/*.c
  ode/src/*.cpp
)
file(GLOB OPCODE_FILES "OPCODE/*.cpp" "OPCODE/Ice/*.cpp" "OPCODE/*.h" "OPCODE/Ice/*.h")
file(GLOB GIMPACT_FILES "GIMPACT/src/*.cpp" "GIMPACT/include/GIMPACT/*.h")
file(GLOB LIBCCD_FILES "libccd/src/ccd/*.h" "libccd/src/*.c")
file(GLOB OU_FILES "ou/include/ou/*.h" "ou/src/ou/*.cpp")

source_group(ode FILES ${ODE_FILES})
source_group(opcode FILES ${OPCODE_FILES})
source_group(gimpact FILES ${GIMPACT_FILES})
source_group(libccd FILES ${LIBCCD_FILES})
source_group(ou FILES ${OU_FILES})

set(SOURCES ${ODE_FILES})
list(REMOVE_ITEM SOURCES ode/src/collision_std.cpp)

if(WITH_OU OR NOT NO_THREADING_INTF)
  include_directories(ou/include)
  set(SOURCES ${SOURCES} ${OU_FILES})
  add_definitions(-D_OU_NAMESPACE=odeou)
endif()

if(NO_DIF)
  list(REMOVE_ITEM SOURCES ode/src/export-dif.cpp)
endif()

if(NO_TRIMESH)
  list(REMOVE_ITEM SOURCES
    ode/src/collision_trimesh_colliders.h
    ode/src/collision_trimesh_internal.h
    ode/src/collision_trimesh_opcode.cpp
    ode/src/collision_trimesh_gimpact.cpp
    ode/src/collision_trimesh_box.cpp
    ode/src/collision_trimesh_ccylinder.cpp
    ode/src/collision_cylinder_trimesh.cpp
    ode/src/collision_trimesh_distance.cpp
    ode/src/collision_trimesh_ray.cpp
    ode/src/collision_trimesh_sphere.cpp
    ode/src/collision_trimesh_trimesh.cpp
    ode/src/collision_trimesh_plane.cpp
  )
elseif(WITH_GIMPACT)
  set(SOURCES ${SOURCES} ${GIMPACT_FILES})
else()
  set(SOURCES ${SOURCES} ${OPCODE_FILES})
endif()

if(WITH_LIBCCD)
  set(SOURCES ${SOURCES} ${LIBCCD_FILES})
  add_definitions(-DdLIBCCD_ENABLED -DdLIBCCD_CYL_CYL)
endif()

if(BUILD_SHARED_LIBS)
  add_definitions(-DODE_DLL)
else()
  add_definitions(-DODE_LIB)
endif()

if(WITH_GIMPACT)
  set(SOURCES ${SOURCES} ${GIMPACT_FILES})
else()
  set(SOURCES ${SOURCES} ${OPCODE_FILES})
endif()

if(MSVC)
  add_library(ode_single ${SOURCES})
  add_library(ode_double ${SOURCES})

  target_compile_definitions(ode_single PUBLIC -DdIDESINGLE -DCCD_IDESINGLE)
  target_compile_definitions(ode_double PUBLIC -DdIDEDOUBLE -DCCD_IDEDOUBLE)

  install(TARGETS ode_single ode_double DESTINATION lib)
else()
  add_library(ode ${SOURCES})

  install(TARGETS ode DESTINATION lib)
endif()

file(GLOB ODE_HEADERS include/ode/*.h)
install(FILES ${ODE_HEADERS} DESTINATION include/ode)
