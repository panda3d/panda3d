if(NOT HAVE_EGL OR NOT HAVE_X11)
  return()
endif()

if(NOT HAVE_GLES1 AND NOT HAVE_GLES2)
  return()
endif()

set(P3EGLDISPLAY_HEADERS
  config_egldisplay.h
  eglGraphicsBuffer.h
  eglGraphicsPipe.h eglGraphicsPipe.I
  eglGraphicsPixmap.h
  eglGraphicsStateGuardian.h eglGraphicsStateGuardian.I
  eglGraphicsWindow.h eglGraphicsWindow.I
)

set(P3EGLDISPLAY_SOURCES
  config_egldisplay.cxx
  eglGraphicsBuffer.cxx
  eglGraphicsPipe.cxx
  eglGraphicsPixmap.cxx
  eglGraphicsStateGuardian.cxx
  eglGraphicsWindow.cxx
)

composite_sources(p3egldisplay P3EGLDISPLAY_SOURCES)

# We actually build this twice!  Once for GLES1, once for GLES2.
if(HAVE_GLES1)
  add_component_library(p3egldisplay_gles1 SYMBOL BUILDING_PANDAGLES
    INIT init_libegldisplay config_egldisplay.h
    ${P3EGLDISPLAY_HEADERS} ${P3EGLDISPLAY_SOURCES})
  target_compile_definitions(p3egldisplay_gles1 PUBLIC OPENGLES_1)
  target_link_libraries(p3egldisplay_gles1 p3glesgsg p3x11display
    PKG::EGL PKG::GLES1)

  if(NOT BUILD_METALIBS)
    install(TARGETS p3egldisplay_gles1 DESTINATION lib)
  endif()
endif()

if(HAVE_GLES2)
  add_component_library(p3egldisplay_gles2 SYMBOL BUILDING_PANDAGLES2
    INIT init_libegldisplay config_egldisplay.h
    ${P3EGLDISPLAY_HEADERS} ${P3EGLDISPLAY_SOURCES})
  target_compile_definitions(p3egldisplay_gles2 PUBLIC OPENGLES_2)
  target_link_libraries(p3egldisplay_gles2 p3gles2gsg p3x11display
    PKG::EGL PKG::GLES2)

  if(NOT BUILD_METALIBS)
    install(TARGETS p3egldisplay_gles2 DESTINATION lib)
  endif()
endif()

install(FILES ${P3EGLDISPLAY_HEADERS} DESTINATION include/panda3d)
