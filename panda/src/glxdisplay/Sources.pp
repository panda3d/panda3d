#define BUILD_DIRECTORY $[HAVE_GLX]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m
#define USE_PACKAGES gl glx cg
#if $[LINK_IN_GLXGETPROCADDRESS]
  #define EXTRA_CDEFS LINK_IN_GLXGETPROCADDRESS
#endif

#begin lib_target
  #define TARGET p3glxdisplay
  #define LOCAL_LIBS \
    p3glgsg p3x11display

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx
 
  #define SOURCES \
    config_glxdisplay.h \
    glxGraphicsBuffer.h glxGraphicsBuffer.I \
    glxGraphicsPipe.h glxGraphicsPipe.I \
    glxGraphicsPixmap.h glxGraphicsPixmap.I \
    glxGraphicsWindow.h \
    glxGraphicsStateGuardian.h glxGraphicsStateGuardian.I \
    posixGraphicsStateGuardian.h posixGraphicsStateGuardian.I \
    panda_glxext.h
 
  #define INCLUDED_SOURCES \
    config_glxdisplay.cxx \
    glxGraphicsBuffer.cxx \
    glxGraphicsPipe.cxx \
    glxGraphicsPixmap.cxx \
    glxGraphicsWindow.cxx \
    glxGraphicsStateGuardian.cxx \
    posixGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    glxGraphicsBuffer.I glxGraphicsBuffer.h \
    glxGraphicsPipe.h glxGraphicsPipe.I \
    glxGraphicsWindow.h

#end lib_target

