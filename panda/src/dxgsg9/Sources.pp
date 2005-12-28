#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolconfig:m \
   dtoolutil:c dtoolbase:c dtool:m

#define WIN_SYS_LIBS \
   d3d9.lib d3dx9.lib dxerr9.lib
   
#define USE_PACKAGES dx

#begin lib_target
  #define TARGET dxgsg9
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian9.cxx \
    dxGraphicsDevice9.h \
    wdxGraphicsPipe9.I wdxGraphicsPipe9.h \
    wdxGraphicsWindow9.I wdxGraphicsWindow9.h \
    dxgsg9base.h config_dxgsg9.h dxGraphicsStateGuardian9.I dxGraphicsStateGuardian9.h \
    dxVertexBufferContext9.h dxVertexbufferContext9.I \
    dxIndexBufferContext9.h dxIndexBufferContext9.I \
    dxTextureContext9.h dxTextureContext9.I \
    dxGeomMunger9.h dxGeomMunger9.I \
    dxGraphicsDevice9.h \
    lru.h
    
  #define INCLUDED_SOURCES \
    config_dxgsg9.cxx \
    dxVertexBufferContext9.cxx \
    dxIndexBufferContext9.cxx \
    dxTextureContext9.cxx \
    dxGeomMunger9.cxx \
    dxGraphicsDevice9.cxx \
    wdxGraphicsPipe9.cxx \
    wdxGraphicsWindow9.cxx \
    wdxGraphicsBuffer9.cxx \
    lru.cxx


#end lib_target
