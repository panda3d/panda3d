if(NOT WIN32)
  return()
endif()

set(P3WINDISPLAY_HEADERS
  config_windisplay.h
  winGraphicsPipe.I winGraphicsPipe.h
  winGraphicsWindow.I winGraphicsWindow.h
  winDetectDx.h
)

set(P3WINDISPLAY_SOURCES
  config_windisplay.cxx winGraphicsPipe.cxx
  winGraphicsWindow.cxx
  winDetectDx9.cxx
)

composite_sources(p3windisplay P3WINDISPLAY_SOURCES)
add_component_library(p3windisplay SYMBOL BUILDING_PANDAWIN
  ${P3WINDISPLAY_HEADERS} ${P3WINDISPLAY_SOURCES})
target_link_libraries(p3windisplay panda user32.lib imm32.lib)

if(HAVE_DX9)
  target_compile_definitions(p3windisplay PRIVATE HAVE_DX9)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3windisplay
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE COMPONENT CoreDevel)
endif()
