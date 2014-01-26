if(HAVE_GL)
  add_definitions(-DBUILDING_PANDAGL)
  set(PANDAGL_LINK_TARGETS p3glgsg)

  add_library(pandagl pandagl.cxx)
  target_link_libraries(pandagl ${PANDAGL_LINK_TARGETS})
endif()
