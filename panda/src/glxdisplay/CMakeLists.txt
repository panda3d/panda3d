if(NOT HAVE_GLX)
  return()
endif()

set(P3GLXDISPLAY_HEADERS
  config_glxdisplay.h
  glxGraphicsBuffer.h glxGraphicsBuffer.I
  glxGraphicsPipe.h glxGraphicsPipe.I
  glxGraphicsPixmap.h glxGraphicsPixmap.I
  glxGraphicsWindow.h glxGraphicsWindow.I
  glxGraphicsStateGuardian.h glxGraphicsStateGuardian.I
  posixGraphicsStateGuardian.h posixGraphicsStateGuardian.I
  panda_glxext.h
)

set(P3GLXDISPLAY_SOURCES
  config_glxdisplay.cxx
  glxGraphicsBuffer.cxx
  glxGraphicsPipe.cxx
  glxGraphicsPixmap.cxx
  glxGraphicsWindow.cxx
  glxGraphicsStateGuardian.cxx
  posixGraphicsStateGuardian.cxx
)

composite_sources(p3glxdisplay P3GLXDISPLAY_SOURCES)
add_component_library(p3glxdisplay SYMBOL BUILDING_PANDA_GLXDISPLAY
  ${P3GLXDISPLAY_HEADERS} ${P3GLXDISPLAY_SOURCES})
target_link_libraries(p3glxdisplay p3glgsg p3x11display)

if(HAVE_X11)
  target_compile_definitions(p3glxdisplay PRIVATE USE_X11)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3glxdisplay EXPORT OpenGL COMPONENT OpenGL DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
