#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#begin lib_target

  #define TARGET dxgsg
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // need to install these due to external projects that link directly with libpandadx (bartop)  
  #define INSTALL_HEADERS \
    config_dxgsg.h dxGraphicsStateGuardian.I dxGraphicsStateGuardian.h \
    dxTextureContext.h dxGeomNodeContext.h dxGeomNodeContext.I dxgsgbase.h

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian.cxx dxSavedFrameBuffer.I dxSavedFrameBuffer.h $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg.cxx dxSavedFrameBuffer.cxx dxTextureContext.cxx dxGeomNodeContext.cxx

#end lib_target
