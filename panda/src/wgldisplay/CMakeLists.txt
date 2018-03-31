if(HAVE_WGL)
  set(P3WGLDISPLAY_HEADERS
      config_wgldisplay.h
      wglGraphicsBuffer.I wglGraphicsBuffer.h
      wglGraphicsPipe.I wglGraphicsPipe.h
      wglGraphicsStateGuardian.I wglGraphicsStateGuardian.h
      wglGraphicsWindow.I wglGraphicsWindow.h
      wglext.h)

  set(P3WGLDISPLAY_SOURCES
      config_wgldisplay.cxx
      wglGraphicsBuffer.cxx
      wglGraphicsPipe.cxx
      wglGraphicsStateGuardian.cxx
      wglGraphicsWindow.cxx)


  composite_sources(p3wgldisplay P3WGLDISPLAY_SOURCES)
  add_library(p3wgldisplay ${P3WGLDISPLAY_HEADERS} ${P3WGLDISPLAY_SOURCES})
  set_target_properties(p3wgldisplay PROPERTIES DEFINE_SYMBOL BUILDING_PANDAGL)
  target_link_libraries(p3wgldisplay p3display p3putil p3windisplay p3glgsg)
endif()
