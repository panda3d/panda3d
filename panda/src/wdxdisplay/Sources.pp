#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#begin lib_target
  #define TARGET wdxdisplay
  #define LOCAL_LIBS \
    dxgsg

  #define SOURCES \
    config_wdxdisplay.cxx config_wdxdisplay.h wdxGraphicsPipe.cxx \
    wdxGraphicsPipe.h wdxGraphicsWindow.cxx wdxGraphicsWindow.h

  #define INSTALL_HEADERS \
    wdxGraphicsPipe.h wdxGraphicsWindow.h

#end lib_target

