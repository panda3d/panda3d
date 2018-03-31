if(HAVE_GLX)
  set(P3GLXDISPLAY_HEADERS
    config_glxdisplay.h
    glxGraphicsBuffer.h glxGraphicsBuffer.I
    glxGraphicsPipe.h glxGraphicsPipe.I
    glxGraphicsPixmap.h glxGraphicsPixmap.I
    glxGraphicsWindow.h
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
  add_library(p3glxdisplay ${P3GLXDISPLAY_HEADERS} ${P3GLXDISPLAY_SOURCES})
  set_target_properties(p3glxdisplay PROPERTIES DEFINE_SYMBOL BUILDING_PANDAGL)
  target_link_libraries(p3glxdisplay p3glgsg p3x11display)

  install(TARGETS p3glxdisplay DESTINATION lib)
endif()
