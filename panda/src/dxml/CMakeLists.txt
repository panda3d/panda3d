set(P3DXML_HEADERS
  config_dxml.h
  tinyxml.h
)

set(P3DXML_SOURCES
  config_dxml.cxx
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

composite_sources(p3dxml P3DXML_SOURCES)
add_definitions(-DTIXML_USE_STL)
add_component_library(p3dxml SYMBOL BUILDING_PANDA_DXML
  ${P3DXML_HEADERS} ${P3DXML_SOURCES})
target_link_libraries(p3dxml pandaexpress)
target_interrogate(p3dxml ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3dxml
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DXML_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
