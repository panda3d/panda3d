#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

// Most of the files here are not actually compiled into anything;
// they're just included by various other directories.

#begin lib_target
  #define TARGET p3glstuff
  #define LOCAL_LIBS \
    p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage

    
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

