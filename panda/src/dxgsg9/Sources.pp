#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES dx

#begin lib_target
  #define TARGET dxgsg9
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // need to install these due to external projects that link directly with libpandadx (bartop)  
  #define INSTALL_HEADERS \
    dxgsg9base.h config_dxgsg9.h dxGraphicsStateGuardian9.I dxGraphicsStateGuardian9.h \
    dxTextureContext9.h dxGeomNodeContext9.h dxGeomNodeContext9.I d3dfont9.h \
    dxGraphicsDevice9.h

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian9.cxx dxSavedFrameBuffer9.I dxSavedFrameBuffer9.h \
    dxGraphicsDevice9.h \
    wdxGraphicsPipe9.I wdxGraphicsPipe9.h \
    wdxGraphicsWindow9.I wdxGraphicsWindow9.h \
    $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg9.cxx \
    dxSavedFrameBuffer9.cxx dxTextureContext9.cxx \
    dxGeomNodeContext9.cxx \
    d3dfont9.cxx \
    dxGraphicsDevice9.cxx \
    wdxGraphicsPipe9.cxx wdxGraphicsWindow9.cxx


#end lib_target
