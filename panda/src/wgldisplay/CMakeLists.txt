if(NOT HAVE_WGL)
  return()
endif()

set(P3WGLDISPLAY_HEADERS
  config_wgldisplay.h
  wglGraphicsBuffer.I wglGraphicsBuffer.h
  wglGraphicsPipe.I wglGraphicsPipe.h
  wglGraphicsStateGuardian.I wglGraphicsStateGuardian.h
  wglGraphicsWindow.I wglGraphicsWindow.h
  wglext.h
)

set(P3WGLDISPLAY_SOURCES
  config_wgldisplay.cxx
  wglGraphicsBuffer.cxx
  wglGraphicsPipe.cxx
  wglGraphicsStateGuardian.cxx
  wglGraphicsWindow.cxx
)

composite_sources(p3wgldisplay P3WGLDISPLAY_SOURCES)
add_component_library(p3wgldisplay SYMBOL BUILDING_PANDA_WGLDISPLAY
  ${P3WGLDISPLAY_HEADERS} ${P3WGLDISPLAY_SOURCES})
target_link_libraries(p3wgldisplay p3glgsg p3windisplay)

if(NOT BUILD_METALIBS)
  install(TARGETS p3wgldisplay
    EXPORT OpenGL COMPONENT OpenGL
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT OpenGLDevel)
endif()
install(FILES ${P3WGLDISPLAY_HEADERS} COMPONENT OpenGLDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
