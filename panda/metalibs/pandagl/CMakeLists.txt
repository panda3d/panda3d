if(HAVE_GL)
  add_definitions(-DBUILDING_PANDAGL)
  set(PANDAGL_LINK_TARGETS p3glgsg)

  if(HAVE_GLX)
    set(PANDAGL_LINK_TARGETS ${PANDAGL_LINK_TARGETS} p3glxdisplay)
  endif()


  add_library(pandagl pandagl.cxx)
  target_link_libraries(pandagl ${PANDAGL_LINK_TARGETS})
endif()
