#define DIRECTORY_IF_GL yes
#define DIRECTORY_IF_GLX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_GL yes

#begin lib_target
  #define TARGET glxdisplay
  #define LOCAL_LIBS \
    glgsg

  #define SOURCES \
    config_glxdisplay.cxx config_glxdisplay.h \
    glxDisplay.I glxDisplay.h glxDisplay.cxx glxGraphicsPipe.cxx \
    glxGraphicsPipe.h glxGraphicsWindow.I glxGraphicsWindow.cxx \
    glxGraphicsWindow.h

  #define INSTALL_HEADERS \
    glxDisplay.I glxDisplay.h \
    glxGraphicsPipe.h glxGraphicsWindow.I glxGraphicsWindow.h

  #define IGATESCAN glxGraphicsPipe.h

#end lib_target

