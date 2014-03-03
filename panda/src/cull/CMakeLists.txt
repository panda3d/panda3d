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
add_library(p3cull ${P3CULL_HEADERS} ${P3CULL_SOURCES})
target_link_libraries(p3cull p3pgraph)
target_interrogate(p3cull ALL)

install(TARGETS p3cull DESTINATION lib)
