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
  ${OPENGL_LIBRARIES})
target_use_packages(p3glgsg CG)

install(TARGETS p3glgsg DESTINATION lib)
install(FILES ${P3GLGSG_HEADERS} DESTINATION include/panda3d)
