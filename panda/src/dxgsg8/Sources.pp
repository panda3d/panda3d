#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES dx

#begin lib_target
  #define TARGET dxgsg8
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // need to install these due to external projects that link directly with libpandadx (bartop)  
  #define INSTALL_HEADERS \
    dxgsg8base.h config_dxgsg8.h dxGraphicsStateGuardian8.I dxGraphicsStateGuardian8.h \
    dxTextureContext8.h dxGeomNodeContext8.h dxGeomNodeContext8.I d3dfont8.h \
    dxGraphicsDevice8.h

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian8.cxx dxSavedFrameBuffer8.I dxSavedFrameBuffer8.h \
    dxGraphicsDevice8.h \
    wdxGraphicsPipe8.I wdxGraphicsPipe8.h \
    wdxGraphicsWindow8.I wdxGraphicsWindow8.h \
    $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg8.cxx \
    dxSavedFrameBuffer8.cxx dxTextureContext8.cxx \
    dxGeomNodeContext8.cxx \
    d3dfont8.cxx \
    dxGraphicsDevice8.cxx \
    wdxGraphicsPipe8.cxx wdxGraphicsWindow8.cxx


#end lib_target
