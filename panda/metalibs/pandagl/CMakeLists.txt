if(HAVE_GL)
  # Check that we actually have a display subsystem.
  if(NOT HAVE_WGL AND NOT HAVE_COCOA AND NOT HAVE_CARBON AND NOT HAVE_GLX)
    message("") # Add extra line before error
    message(SEND_ERROR
      "When compiling with OpenGL (HAVE_GL), at least one of:
    HAVE_WGL, HAVE_COCOA, HAVE_CARBON, or HAVE_GLX must be defined.")
  endif()

  add_definitions(-DBUILDING_PANDAGL)
  set(PANDAGL_LINK_TARGETS p3glgsg)

  if(HAVE_GLX)
    set(PANDAGL_LINK_TARGETS ${PANDAGL_LINK_TARGETS} p3glxdisplay)
  endif()

  if(HAVE_COCOA)
    set(PANDAGL_LINK_TARGETS ${PANDAGL_LINK_TARGETS} p3cocoadisplay)
  endif()

  if(HAVE_CARBON)
    set(PANDAGL_LINK_TARGETS ${PANDAGL_LINK_TARGETS} p3osxdisplay)
  endif()

  add_library(pandagl pandagl.cxx)
  target_link_libraries(pandagl ${PANDAGL_LINK_TARGETS})

  install(TARGETS pandagl DESTINATION lib)
endif()
