#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

// Most of the files here are not actually compiled into anything;
// they're just included by various other directories.

#begin lib_target
  #define TARGET glstuff
  #define LOCAL_LIBS \
    gsgbase gobj display \
    putil linmath mathutil pnmimage \
    effects

    
  #define INSTALL_HEADERS \
     glGeomContext_src.I \
     glGeomContext_src.cxx \
     glGeomContext_src.h \
     glGeomMunger_src.I \
     glGeomMunger_src.cxx \
     glGeomMunger_src.h \
     glGraphicsStateGuardian_src.I \
     glGraphicsStateGuardian_src.cxx \
     glGraphicsStateGuardian_src.h \
     glGraphicsBuffer_src.I \
     glGraphicsBuffer_src.cxx \
     glGraphicsBuffer_src.h \
     glImmediateModeSender_src.I \
     glImmediateModeSender_src.cxx \
     glImmediateModeSender_src.h \
     glIndexBufferContext_src.I \
     glIndexBufferContext_src.cxx \
     glIndexBufferContext_src.h \
     glOcclusionQueryContext_src.I \
     glOcclusionQueryContext_src.cxx \
     glOcclusionQueryContext_src.h \
     glShaderContext_src.I \
     glShaderContext_src.cxx \
     glShaderContext_src.h \
     glTextureContext_src.I \
     glTextureContext_src.cxx \
     glTextureContext_src.h \
     glVertexBufferContext_src.I \
     glVertexBufferContext_src.cxx \
     glVertexBufferContext_src.h \
     glmisc_src.cxx \
     glmisc_src.h \
     glstuff_src.cxx \
     glstuff_src.h \
     glstuff_undef_src.h \
     panda_glext.h

  #define SOURCES \
    $[INSTALL_HEADERS] glpure.cxx

#end lib_target

