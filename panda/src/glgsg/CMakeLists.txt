if(NOT HAVE_GL)
  return()
endif()

set(P3GLGSG_HEADERS
  config_glgsg.h glgsg.h
)

set(P3GLGSG_SOURCES
  config_glgsg.cxx
  glgsg.cxx
)

composite_sources(p3glgsg P3GLGSG_SOURCES)
add_component_library(p3glgsg SYMBOL BUILDING_PANDA_GLGSG
  ${P3GLGSG_HEADERS} ${P3GLGSG_SOURCES})
target_link_libraries(p3glgsg p3glstuff panda
  PKG::CG PKG::GL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3glgsg
    EXPORT OpenGL COMPONENT OpenGL
    DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE COMPONENT OpenGLDevel)
endif()
install(FILES ${P3GLGSG_HEADERS} COMPONENT OpenGLDevel DESTINATION include/panda3d)
