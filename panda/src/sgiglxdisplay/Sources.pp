#define DIRECTORY_IF_SGIGL yes
#define DIRECTORY_IF_GLX yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET sgiglxdisplay
  #define LOCAL_LIBS \
    sgidisplay glxdisplay

  #define SOURCES \
    config_sgiglxdisplay.cxx config_sgiglxdisplay.h \
    sgiglxGraphicsPipe.cxx sgiglxGraphicsPipe.h

  #define INSTALL_HEADERS \
    sgiglxGraphicsPipe.h

#end lib_target

