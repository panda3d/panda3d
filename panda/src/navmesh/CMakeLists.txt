set(NAVMESH_SOURCES
  config_navmesh.cxx
  config_navmesh.h
  navMesh.cxx
  navMesh.h
  navMeshBuilder.cxx
  navMeshBuilder.h
  navMeshSettings.cxx
  navMeshSettings.h
  navMeshSettings.I
  navPath.cxx
  navPath.h
  navPath.I
)

composite_sources(navmesh NAVMESH_SOURCES)
add_library(navmesh ${NAVMESH_SOURCES})
target_link_libraries(navmesh panda)
target_interrogate(navmesh ALL)

install(TARGETS navmesh
  EXPORT Contrib COMPONENT NavMesh
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT NavMeshDevel)
install(FILES ${NAVMESH_HEADERS} COMPONENT NavMeshDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

