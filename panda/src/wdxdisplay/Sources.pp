#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET wdxdisplay

  #define SOURCES \
    config_wdxdisplay.cxx config_wdxdisplay.h wdxGraphicsPipe.cxx \
    wdxGraphicsPipe.h wdxGraphicsWindow.cxx wdxGraphicsWindow.h

  #define INSTALL_HEADERS \
    wdxGraphicsPipe.h wdxGraphicsWindow.h

#end lib_target

