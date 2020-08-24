if(NOT HAVE_GL)
  return()
endif()

set(P3GLSTUFF_HEADERS
  glGeomContext_src.I
  glGeomContext_src.h
  glGeomMunger_src.I
  glGeomMunger_src.h
  glGraphicsStateGuardian_src.I
  glGraphicsStateGuardian_src.h
  glGraphicsBuffer_src.I
  glGraphicsBuffer_src.h
  glImmediateModeSender_src.I
  glImmediateModeSender_src.h
  glIndexBufferContext_src.I
  glIndexBufferContext_src.h
  glOcclusionQueryContext_src.I
  glOcclusionQueryContext_src.h
  glShaderContext_src.I
  glShaderContext_src.h
  glTextureContext_src.I
  glTextureContext_src.h
  glVertexBufferContext_src.I
  glVertexBufferContext_src.h
  glmisc_src.h
  glstuff_src.h
  glstuff_undef_src.h
  panda_glext.h
)

set(P3GLSTUFF_SOURCES
  glpure.cxx
)

composite_sources(p3glstuff P3GLSTUFF_SOURCES)
add_component_library(p3glstuff NOINIT ${P3GLSTUFF_HEADERS} ${P3GLSTUFF_SOURCES})
target_link_libraries(p3glstuff panda)

if(APPLE)
  # We acknowledge Apple's hostility to open standards so that the SDK won't
  # bother our users about it.
  target_compile_definitions(p3glstuff PUBLIC GL_SILENCE_DEPRECATION)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3glstuff
    EXPORT OpenGL COMPONENT OpenGL
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT OpenGLDevel)
endif()
install(FILES ${P3GLSTUFF_HEADERS} COMPONENT OpenGLDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
