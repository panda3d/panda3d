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
add_component_library(p3glstuff ${P3GLSTUFF_HEADERS} ${P3GLSTUFF_SOURCES})
target_link_libraries(p3glstuff panda)

install(TARGETS p3glstuff DESTINATION lib)
install(FILES ${P3GLSTUFF_HEADERS} DESTINATION include/panda3d)
