#define DIRECTORY_IF_GL yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m
#define USE_GL yes

#begin lib_target
  #define TARGET glgsg
  #define LOCAL_LIBS \
    cull gsgmisc gsgbase gobj sgattrib sgraphutil graph display light \
    putil linmath sgraph mathutil pnmimage

  #define SOURCES \
    config_glgsg.cxx config_glgsg.h glGraphicsStateGuardian.I \
    glGraphicsStateGuardian.cxx glGraphicsStateGuardian.h \
    glSavedFrameBuffer.I glSavedFrameBuffer.cxx glSavedFrameBuffer.h \
    glTextureContext.I glTextureContext.cxx glTextureContext.h

  #define INSTALL_HEADERS \
    config_glgsg.h glGraphicsStateGuardian.I glGraphicsStateGuardian.h

#end lib_target

