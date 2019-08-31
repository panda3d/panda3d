set(P3GSGBASE_HEADERS
  config_gsgbase.h
  graphicsOutputBase.I graphicsOutputBase.h
  graphicsStateGuardianBase.h
)

set(P3GSGBASE_SOURCES
  config_gsgbase.cxx
  graphicsOutputBase.cxx
  graphicsStateGuardianBase.cxx
)

composite_sources(p3gsgbase P3GSGBASE_SOURCES)
add_component_library(p3gsgbase NOINIT SYMBOL BUILDING_PANDA_GSGBASE
  ${P3GSGBASE_HEADERS} ${P3GSGBASE_SOURCES})
target_link_libraries(p3gsgbase p3putil p3linmath)
target_interrogate(p3gsgbase ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3gsgbase
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3GSGBASE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
