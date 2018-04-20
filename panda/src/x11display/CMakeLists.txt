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
add_library(p3x11display ${P3X11DISPLAY_HEADERS} ${P3X11DISPLAY_SOURCES})
set_target_properties(p3x11display PROPERTIES DEFINE_SYMBOL BUILDING_PANDAX11)
target_link_libraries(p3x11display panda ${X11_LIBRARIES})

install(TARGETS p3x11display DESTINATION lib)
