
# Define header files
set(NAV_MESH_HEADERS
    config_navmesh.h
    navMeshSettings.h navMeshSettings.I
    navMeshBuilder.h
    navMesh.h
    navPath.h navPath.I
)

# Define source files
set(NAV_MESH_SOURCES
    config_navmesh.cxx
    navMeshSettings.cxx
    navMeshBuilder.cxx
    navMesh.cxx
    navPath.cxx
)

set(RECAST_SOURCES
    thirdparty/Recast/Source/Recast.cpp
    thirdparty/Recast/Source/RecastAlloc.cpp
    thirdparty/Recast/Source/RecastArea.cpp
    thirdparty/Recast/Source/RecastContour.cpp
    thirdparty/Recast/Source/RecastFilter.cpp
    thirdparty/Recast/Source/RecastLayers.cpp
    thirdparty/Recast/Source/RecastMesh.cpp
    thirdparty/Recast/Source/RecastMeshDetail.cpp
    thirdparty/Recast/Source/RecastRasterization.cpp
    thirdparty/Recast/Source/RecastRegion.cpp
    thirdparty/Detour/Source/DetourNavMesh.cpp
    thirdparty/Detour/Source/DetourNavMeshBuilder.cpp
    thirdparty/Detour/Source/DetourNavMeshQuery.cpp
    thirdparty/Detour/Source/DetourNode.cpp
    thirdparty/Detour/Source/DetourAlloc.cpp
    thirdparty/Detour/Source/DetourCommon.cpp
)

# Recast sources must not be unified because they contain conflicting internal symbols
set_source_files_properties(${RECAST_SOURCES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

# SEPARATION FIX: Build Recast as a separate static lib to hide it from Interrogate
add_library(p3recast STATIC ${RECAST_SOURCES})
target_include_directories(p3recast PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Recast/Include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Detour/Include>
)
set_target_properties(p3recast PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(p3recast PROPERTIES OUTPUT_NAME p3recast)

# Create the library (composite_sources modifies the sources list, but we still need add_library)
composite_sources(p3navmesh NAV_MESH_SOURCES)
# Remove RECAST_SOURCES from this list
add_library(p3navmesh ${NAV_MESH_SOURCES} ${NAV_MESH_HEADERS})

# Include directories
target_include_directories(p3navmesh PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/panda3d>
)

# Link against Panda3D core libraries (Metalibs) and our new p3recast
target_link_libraries(p3navmesh PUBLIC p3dtool p3prc p3dconfig pandaexpress panda p3recast)

# Define building symbol for export macros
set_target_properties(p3navmesh PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_NAVMESH)

# Python Bindings
target_interrogate(p3navmesh ALL)

# Install and export
install(TARGETS p3recast
  EXPORT NavMesh COMPONENT NavMesh
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT NavMeshDevel)
install(TARGETS p3navmesh
  EXPORT NavMesh COMPONENT NavMesh
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT NavMeshDevel)
install(FILES ${NAV_MESH_HEADERS} COMPONENT NavMeshDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
