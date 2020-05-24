if(NOT HAVE_X11)
  return()
endif()

set(P3X11DISPLAY_HEADERS
  config_x11display.h x11GraphicsPipe.I x11GraphicsPipe.h
  x11GraphicsWindow.h x11GraphicsWindow.I
)

set(P3X11DISPLAY_SOURCES
  config_x11display.cxx x11GraphicsPipe.cxx x11GraphicsWindow.cxx
)

composite_sources(p3x11display P3X11DISPLAY_SOURCES)
add_component_library(p3x11display SYMBOL BUILDING_PANDAX11
  ${P3X11DISPLAY_HEADERS} ${P3X11DISPLAY_SOURCES})
target_link_libraries(p3x11display panda PKG::X11)

if(NOT BUILD_METALIBS)
  install(TARGETS p3x11display EXPORT Core COMPONENT Core DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
