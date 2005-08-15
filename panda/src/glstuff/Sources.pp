#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl cggl
// Most of the files here are not actually compiled into anything;
// they're just included by various other directories.

#begin lib_target
  #define TARGET glstuff
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage \
    effects

    
  #define INSTALL_HEADERS \
     panda_glext.h \
     glmisc_src.cxx \
     glmisc_src.h \
     glstuff_src.cxx \
     glstuff_src.h \
     glstuff_undef_src.h \
     glVertexBufferContext_src.cxx \
     glVertexBufferContext_src.I \
     glVertexBufferContext_src.h \
     glImmediateModeSender_src.cxx \
     glImmediateModeSender_src.I \
     glImmediateModeSender_src.h \
     glIndexBufferContext_src.cxx \
     glIndexBufferContext_src.I \
     glIndexBufferContext_src.h \
     glGeomContext_src.cxx \
     glGeomContext_src.I \
     glGeomContext_src.h \
     glGeomMunger_src.cxx \
     glGeomMunger_src.I \
     glGeomMunger_src.h \
     glGraphicsStateGuardian_src.cxx \
     glGraphicsStateGuardian_src.I \
     glGraphicsStateGuardian_src.h \
     glTextureContext_src.cxx \
     glTextureContext_src.I \
     glTextureContext_src.h \
     glCgShaderContext_src.cxx \
     glCgShaderContext_src.h \
     glCgShaderContext_src.I \

  #define SOURCES \
    $[INSTALL_HEADERS] glpure.cxx

#end lib_target

