#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#begin lib_target
  #define TARGET dxgsg

  #define SOURCES \
    config_dxgsg.cxx config_dxgsg.h dxGraphicsStateGuardian.I \
    dxGraphicsStateGuardian.cxx dxGraphicsStateGuardian.h \
    dxSavedFrameBuffer.I dxSavedFrameBuffer.cxx dxSavedFrameBuffer.h \
    dxTextureContext.I dxTextureContext.cxx dxTextureContext.h

  #define INSTALL_HEADERS \
    config_dxgsg.h dxGraphicsStateGuardian.I dxGraphicsStateGuardian.h \
    dxSavedFrameBuffer.I dxSavedFrameBuffer.h dxTextureContext.I \
    dxTextureContext.h

#end lib_target
