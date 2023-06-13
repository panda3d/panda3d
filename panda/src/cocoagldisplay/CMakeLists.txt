if(NOT APPLE OR NOT HAVE_GL OR NOT HAVE_COCOA)
  return()
endif()

set(P3COCOAGLDISPLAY_HEADERS
  cocoaGLGraphicsBuffer.h cocoaGLGraphicsBuffer.I
  cocoaGLGraphicsPipe.h cocoaGLGraphicsPipe.I
  cocoaGLGraphicsStateGuardian.h cocoaGLGraphicsStateGuardian.I
  cocoaGLGraphicsWindow.h cocoaGLGraphicsWindow.I
  config_cocoagldisplay.h
)

set(P3COCOAGLDISPLAY_SOURCES
  cocoaGLGraphicsBuffer.mm
  cocoaGLGraphicsPipe.mm
  cocoaGLGraphicsStateGuardian.mm
  cocoaGLGraphicsWindow.mm
  config_cocoagldisplay.mm
)

composite_sources(p3cocoagldisplay P3COCOAGLDISPLAY_SOURCES)
add_component_library(p3cocoagldisplay SYMBOL BUILDING_PANDA_COCOAGLDISPLAY
  ${P3COCOAGLDISPLAY_HEADERS} ${P3COCOAGLDISPLAY_SOURCES})
target_link_libraries(p3cocoagldisplay p3cocoadisplay p3glgsg panda)

if(NOT BUILD_METALIBS)
  install(TARGETS p3cocoagldisplay EXPORT OpenGL COMPONENT OpenGL DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
