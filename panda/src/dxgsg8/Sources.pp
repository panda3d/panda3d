#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#if $[BUILD_DX8]
#begin lib_target

  #define TARGET dxgsg8
  #define LOCAL_LIBS \
    cull gsgmisc gsgbase gobj sgattrib sgraphutil graph display light \
    putil linmath sgraph mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_dxgsg8.h dxGraphicsStateGuardian8.I \
    dxGraphicsStateGuardian8.cxx dxGraphicsStateGuardian8.h \
    dxSavedFrameBuffer8.I dxSavedFrameBuffer8.h \
    dxTextureContext8.h dxGeomNodeContext8.h dxGeomNodeContext8.I
    
  #define INCLUDED_SOURCES \
    config_dxgsg8.cxx dxSavedFrameBuffer8.cxx dxTextureContext8.cxx dxGeomNodeContext8.cxx
    
  #define INSTALL_HEADERS \
    dxGraphicsStateGuardian8.I dxGraphicsStateGuardian8.h \
    dxSavedFrameBuffer8.I dxSavedFrameBuffer8.h dxTextureContext8.h

#end lib_target
#endif
