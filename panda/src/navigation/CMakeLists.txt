set(P3NAVIGATION_HEADERS
        config_navigation.h
        navMesh.h
        navMesh.I
        navMeshNode.h
        navMeshNode.I
        navMeshQuery.h
        navMeshQuery.I
        navMeshPath.h
        navMeshPath.I
        )

set(P3NAVIGATION_SOURCES
        config_navigation.cxx
        navMesh.cxx
        navMeshNode.cxx
        navMeshQuery.cxx
        navMeshPath.cxx
        )

file(GLOB P3NAVIGATION_HEADERS *.h *.I)
file(GLOB P3NAVIGATION_SOURCES *.cpp ${P3NAVIGATION_SOURCES})

composite_sources(p3navigation P3NAVIGATION_SOURCES)
add_library(p3navigation ${P3NAVIGATION_HEADERS} ${P3NAVIGATION_SOURCES})
set_target_properties(p3navigation PROPERTIES DEFINE_SYMBOL BUILDING_NAVIGATION)
target_link_libraries(p3navigation panda)
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
