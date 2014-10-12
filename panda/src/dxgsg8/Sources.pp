#define BUILD_DIRECTORY $[HAVE_DX8]

#define OTHER_LIBS \
   p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
   p3dtoolutil:c p3dtoolbase:c p3dtool:m
   
#define USE_PACKAGES dx8

#begin lib_target
  #define TARGET p3dxgsg8
  #define LOCAL_LIBS \
    p3gsgbase p3gobj p3display p3windisplay \
    p3putil p3linmath p3mathutil p3pnmimage p3event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  // build dxGraphicsStateGuardian separately since its so big
  
  #define SOURCES \
    dxGraphicsStateGuardian8.cxx \
    dxGraphicsDevice8.h \
    wdxGraphicsPipe8.I wdxGraphicsPipe8.h \
    wdxGraphicsWindow8.I wdxGraphicsWindow8.h \
    dxgsg8base.h config_dxgsg8.h dxGraphicsStateGuardian8.I dxGraphicsStateGuardian8.h \
    dxVertexBufferContext8.h dxVertexBufferContext8.I \
    dxIndexBufferContext8.h dxIndexBufferContext8.I \
    dxTextureContext8.h dxTextureContext8.I \
    dxGeomMunger8.h dxGeomMunger8.I \
    dxGraphicsDevice8.h
    
  #define INCLUDED_SOURCES \
    config_dxgsg8.cxx \
    dxVertexBufferContext8.cxx \
    dxIndexBufferContext8.cxx \
    dxTextureContext8.cxx \
    dxGeomMunger8.cxx \
    dxGraphicsDevice8.cxx \
    wdxGraphicsPipe8.cxx \
    wdxGraphicsWindow8.cxx \
    wdxGraphicsBuffer8.cxx

#end lib_target
