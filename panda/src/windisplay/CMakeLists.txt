if(NOT WIN32)
  return()
endif()

set(P3WINDISPLAY_HEADERS
    config_windisplay.h
    winGraphicsPipe.I winGraphicsPipe.h
    winGraphicsWindow.I winGraphicsWindow.h
    winDetectDx.h)

set(P3WINDISPLAY_SOURCES
    config_windisplay.cxx winGraphicsPipe.cxx
    winGraphicsWindow.cxx
    winDetectDx9.cxx)

composite_sources(p3windisplay P3WINDISPLAY_SOURCES)
add_library(p3windisplay ${P3WINDISPLAY_HEADERS} ${P3WINDISPLAY_SOURCES})
set_target_properties(p3windisplay PROPERTIES DEFINE_SYMBOL BUILDING_PANDAWIN)
target_link_libraries(p3windisplay p3display p3putil Imm32.lib)
