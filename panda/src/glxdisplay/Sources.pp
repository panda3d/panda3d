#define DIRECTORY_IF_GL yes
#define DIRECTORY_IF_GLX yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase
#define USE_GL yes

#begin lib_target
  #define TARGET glxdisplay
  #define LOCAL_LIBS \
    glgsg

  #define SOURCES \
    config_glxdisplay.cxx config_glxdisplay.h glxGraphicsPipe.cxx \
    glxGraphicsPipe.h glxGraphicsWindow.I glxGraphicsWindow.cxx \
    glxGraphicsWindow.h

  #define INSTALL_HEADERS \
    glxGraphicsPipe.h glxGraphicsWindow.I glxGraphicsWindow.h

#end lib_target

