set(P3NAVMESHGEN_HEADERS
        config_navmeshgen.h
        navMeshBuilder.h
        )

set(P3NAVMESHGEN_SOURCES
        config_navmeshgen.cxx
        navMeshBuilder.cxx
        )

file(GLOB P3NAVMESHGEN_HEADERS *.h *.I)
file(GLOB P3NAVMESHGEN_SOURCES *.cpp ${P3NAVMESHGEN_SOURCES})

composite_sources(p3navmeshgen P3NAVMESHGEN_SOURCES)
add_library(p3navmeshgen ${P3NAVMESHGEN_HEADERS} ${P3NAVMESHGEN_SOURCES})
set_target_properties(p3navmeshgen PROPERTIES DEFINE_SYMBOL BUILDING_NAVMESHGEN)
target_link_libraries(p3navmeshgen panda p3navigation)
set_target_properties(p3navmeshgen PROPERTIES CXX_EXCEPTIONS ON)
target_interrogate(p3navmeshgen ALL)

install(TARGETS p3navmeshgen
        EXPORT NavMeshGen COMPONENT NavMeshGen
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
        ARCHIVE COMPONENT NavMeshGenDevel)
install(FILES ${P3NAVMESHGEN_HEADERS} COMPONENT NavMeshGenDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(NavMeshGen COMPONENT NavMeshGen)
