set(P3DGRAPH_HEADERS
  config_dgraph.h
  dataGraphTraverser.I dataGraphTraverser.h
  dataNode.I dataNode.h
  dataNodeTransmit.I dataNodeTransmit.h
)

set(P3DGRAPH_SOURCES
  config_dgraph.cxx
  dataGraphTraverser.cxx
  dataNode.cxx
  dataNodeTransmit.cxx
)

composite_sources(p3dgraph P3DGRAPH_SOURCES)
add_component_library(p3dgraph NOINIT SYMBOL BUILDING_PANDA_DGRAPH
  ${P3DGRAPH_HEADERS} ${P3DGRAPH_SOURCES})
target_link_libraries(p3dgraph p3pgraph)
target_interrogate(p3dgraph ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3dgraph
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DGRAPH_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
