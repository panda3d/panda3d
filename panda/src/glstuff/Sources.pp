#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

// Most of the files here are not actually compiled into anything;
// they're just included by various other directories.

#begin lib_target
  #define TARGET glstuff
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage
    
  #define INSTALL_HEADERS \
     glext.h \
     glstuff_src.cxx \
     glstuff_src.h \
     glstuff_undef_src.h \
     glGraphicsStateGuardian_src.cxx \
     glGraphicsStateGuardian_src.I \
     glGraphicsStateGuardian_src.h \
     glSavedFrameBuffer_src.cxx \
     glSavedFrameBuffer_src.I \
     glSavedFrameBuffer_src.h \
     glTextureContext_src.cxx \
     glTextureContext_src.I \
     glTextureContext_src.h \
     glGeomNodeContext_src.cxx \
     glGeomNodeContext_src.I \
     glGeomNodeContext_src.h

  #define SOURCES \
    $[INSTALL_HEADERS] glpure.cxx

#end lib_target

