#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolconfig:m \
   dtoolutil:c dtoolbase:c dtool:m

#define WIN_SYS_LIBS \
   d3d8.lib d3dx8.lib dxerr8.lib
   
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
    dxVertexBufferContext8.h \
    dxTextureContext8.h \
    dxGeomMunger8.h \
    d3dfont8.h \
    dxGraphicsDevice8.h

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian8.cxx \
    dxGraphicsDevice8.h \
    wdxGraphicsPipe8.I wdxGraphicsPipe8.h \
    wdxGraphicsWindow8.I wdxGraphicsWindow8.h \
    $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg8.cxx \
    dxVertexBufferContext8.cxx \
    dxTextureContext8.cxx \
    dxGeomMunger8.cxx \
    d3dfont8.cxx \
    dxGraphicsDevice8.cxx \
    wdxGraphicsPipe8.cxx wdxGraphicsWindow8.cxx


#end lib_target
