if(NOT HAVE_RECAST OR NOT HAVE_DETOUR)
    return()
endif()

set(P3NAVIGATION_HEADERS
        config_navigation.h
        navMesh.h
        navMesh.I
        navMeshNode.h
        navMeshNode.I
        navMeshPath.h
        navMeshPath.I
        navMeshPoly.h
        navMeshPoly.I
        navMeshQuery.h
        navMeshQuery.I
        navMeshQueryFilter.h
        navMeshQueryFilter.I
        )

set(P3NAVIGATION_SOURCES
        config_navigation.cxx
        navMesh.cxx
        navMeshNode.cxx
        navMeshPath.cxx
        navMeshPoly.cxx
        navMeshQuery.cxx
        navMeshQueryFilter.cxx
        )

composite_sources(p3navigation P3NAVIGATION_SOURCES)
add_library(p3navigation ${P3NAVIGATION_HEADERS} ${P3NAVIGATION_SOURCES})
set_target_properties(p3navigation PROPERTIES DEFINE_SYMBOL BUILDING_NAVIGATION)
target_link_libraries(p3navigation panda
  PKG::RECAST
  PKG::DETOUR)
set_target_properties(p3navigation PROPERTIES CXX_EXCEPTIONS ON)
target_interrogate(p3navigation ALL)

install(TARGETS p3navigation
        EXPORT Navigation COMPONENT Navigation
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
        ARCHIVE COMPONENT NavigationDevel)
install(FILES ${P3NAVIGATION_HEADERS} COMPONENT NavigationDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(Navigation COMPONENT NavigationDevel)
