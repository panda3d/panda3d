#define DIRECTORY_IF_GL yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_GL yes

#begin lib_target
  #define TARGET glgsg
  #define LOCAL_LIBS \
    cull gsgmisc gsgbase gobj sgattrib sgraphutil graph display light \
    putil linmath sgraph mathutil pnmimage
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
     glGraphicsStateGuardian.cxx \
     config_glgsg.h glGraphicsStateGuardian.I \
     glGraphicsStateGuardian.h glSavedFrameBuffer.I \
     glSavedFrameBuffer.h glTextureContext.I \
     glGeomNodeContext.I glGeomNodeContext.h  glTextureContext.h

  #define INCLUDED_SOURCES \
     config_glgsg.cxx glSavedFrameBuffer.cxx \
     glGeomNodeContext.cxx glTextureContext.cxx 


  #define INSTALL_HEADERS \
    config_glgsg.h glGraphicsStateGuardian.I glGraphicsStateGuardian.h

#end lib_target

