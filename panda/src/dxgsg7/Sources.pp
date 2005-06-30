// DX7 build is temporarily disabled until we bring it up-to-date with
// the new Geom rewrite.
#define BUILD_DIRECTORY
//#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES dx

// We link with the DX8 libraries, because they're a superset of DX7
// anyway, and it means we don't need to have the DX7 SDK available.
#define WIN_SYS_LIBS \
   d3d8.lib d3dx8.lib dxerr8.lib

#begin lib_target

  #define TARGET dxgsg7
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // need to install these due to external projects that link directly with libpandadx (bartop)  
  #define INSTALL_HEADERS \
    config_dxgsg7.h dxGraphicsStateGuardian7.I dxGraphicsStateGuardian7.h \
    dxTextureContext7.h dxgsg7base.h

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian7.cxx \
    wdxGraphicsPipe7.I wdxGraphicsPipe7.h \
    wdxGraphicsWindow7.I wdxGraphicsWindow7.h \
    $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg7.cxx \
    dxgsg7base.cxx \
    dxTextureContext7.cxx \
    wdxGraphicsPipe7.cxx wdxGraphicsWindow7.cxx

#end lib_target
