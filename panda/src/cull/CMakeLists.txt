set(P3CULL_HEADERS
  binCullHandler.h binCullHandler.I
  config_cull.h
  cullBinBackToFront.h cullBinBackToFront.I
  cullBinFixed.h cullBinFixed.I
  cullBinFrontToBack.h cullBinFrontToBack.I
  cullBinStateSorted.h cullBinStateSorted.I
  cullBinUnsorted.h cullBinUnsorted.I
  drawCullHandler.h drawCullHandler.I
)

set(P3CULL_SOURCES
  binCullHandler.cxx
  config_cull.cxx
  cullBinBackToFront.cxx
  cullBinFixed.cxx
  cullBinFrontToBack.cxx
  cullBinStateSorted.cxx
  cullBinUnsorted.cxx
  drawCullHandler.cxx
)

composite_sources(p3cull P3CULL_SOURCES)
add_component_library(p3cull SYMBOL BUILDING_PANDA_CULL
  ${P3CULL_HEADERS} ${P3CULL_SOURCES})
target_link_libraries(p3cull p3pgraph)
target_interrogate(p3cull ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3cull
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3CULL_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
