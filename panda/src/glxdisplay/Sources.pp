#define BUILD_DIRECTORY $[HAVE_GLX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl glx cg
#if $[LINK_IN_GLXGETPROCADDRESS]
  #define EXTRA_CDEFS LINK_IN_GLXGETPROCADDRESS
#endif

#begin lib_target
  #define TARGET glxdisplay
  #define LOCAL_LIBS \
    glgsg x11display

  #define SOURCES \
    config_glxdisplay.cxx config_glxdisplay.h \
    glxGraphicsBuffer.h glxGraphicsBuffer.I glxGraphicsBuffer.cxx \
    glxGraphicsPipe.cxx glxGraphicsPipe.h \
    glxGraphicsPixmap.h glxGraphicsPixmap.I glxGraphicsPixmap.cxx \
    glxGraphicsWindow.h glxGraphicsWindow.cxx \
    glxGraphicsStateGuardian.h glxGraphicsStateGuardian.I \
    glxGraphicsStateGuardian.cxx \
    panda_glxext.h

  #define INSTALL_HEADERS \
    glxGraphicsBuffer.I glxGraphicsBuffer.h \
    glxGraphicsPipe.h glxGraphicsWindow.h

#end lib_target

