set(P3PANDATOOLBASE_HEADERS
  animationConvert.h
  config_pandatoolbase.h
  distanceUnit.h
  pandatoolbase.h pandatoolsymbols.h
  pathReplace.h pathReplace.I
  pathStore.h
)

set(P3PANDATOOLBASE_SOURCES
  animationConvert.cxx
  config_pandatoolbase.cxx
  distanceUnit.cxx
  pandatoolbase.cxx
  pathReplace.cxx
  pathStore.cxx
)

composite_sources(p3pandatoolbase P3PANDATOOLBASE_SOURCES)
add_library(p3pandatoolbase STATIC ${P3PANDATOOLBASE_HEADERS} ${P3PANDATOOLBASE_SOURCES})
target_link_libraries(p3pandatoolbase panda)

# ptloader - a module - may need to link in libraries in this package, so:
if(MODULE_TYPE STREQUAL "MODULE")
  set_target_properties(p3pandatoolbase PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    INTERFACE_POSITION_INDEPENDENT_CODE ON)
endif()

install(TARGETS p3pandatoolbase
  EXPORT ToolsDevel COMPONENT ToolsDevel
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ToolsDevel)
install(FILES ${P3PANDATOOLBASE_HEADERS} COMPONENT ToolsDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
