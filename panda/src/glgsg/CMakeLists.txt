if(HAVE_GL)
  set(P3GLGSG_HEADERS
    config_glgsg.h glgsg.h
  )

  set(P3GLGSG_SOURCES
    config_glgsg.cxx
    glgsg.cxx
  )

  composite_sources(p3glgsg P3GLGSG_SOURCES)
  add_library(p3glgsg ${P3GLGSG_HEADERS} ${P3GLGSG_SOURCES})
  set_target_properties(p3glgsg PROPERTIES DEFINE_SYMBOL BUILDING_PANDAGL)
  target_link_libraries(p3glgsg p3glstuff p3display
    ${OPENGL_LIBRARIES})
  target_use_packages(p3glgsg CG)

  install(TARGETS p3glgsg DESTINATION lib)
  install(FILES ${P3GLGSG_HEADERS} DESTINATION include/panda3d)
endif()
