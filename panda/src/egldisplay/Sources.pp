#define BUILD_DIRECTORY $[and $[HAVE_EGL],$[HAVE_X11]]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m

#begin lib_target
  #define TARGET p3egldisplay
  #define BUILD_TARGET $[HAVE_GLES]
  #define USE_PACKAGES gles egl x11
  #define EXTRA_CDEFS OPENGLES_1
  #define LOCAL_LIBS \
    p3glesgsg p3x11display

  #define SOURCES \
    config_egldisplay.cxx config_egldisplay.h \
    eglGraphicsBuffer.h eglGraphicsBuffer.cxx \
    eglGraphicsPipe.I eglGraphicsPipe.cxx eglGraphicsPipe.h \
    eglGraphicsPixmap.h eglGraphicsPixmap.cxx \
    eglGraphicsWindow.h eglGraphicsWindow.cxx \
    eglGraphicsStateGuardian.h eglGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    eglGraphicsBuffer.h eglGraphicsPixmap.h \
    eglGraphicsPipe.I eglGraphicsPipe.h \
    eglGraphicsWindow.I eglGraphicsWindow.h

#end lib_target

#begin lib_target
  #define TARGET egl2display
  #define BUILD_TARGET $[HAVE_GLES2]
  #define USE_PACKAGES gles2 egl x11
  #define EXTRA_CDEFS OPENGLES_2
  #define LOCAL_LIBS \
    p3gles2gsg p3x11display

  #define SOURCES \
    config_egldisplay.cxx config_egldisplay.h \
    eglGraphicsBuffer.h eglGraphicsBuffer.cxx \
    eglGraphicsPipe.I eglGraphicsPipe.cxx eglGraphicsPipe.h \
    eglGraphicsPixmap.h eglGraphicsPixmap.cxx \
    eglGraphicsWindow.h eglGraphicsWindow.cxx \
    eglGraphicsStateGuardian.h eglGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    eglGraphicsBuffer.h eglGraphicsPixmap.h \
    eglGraphicsPipe.I eglGraphicsPipe.h \
    eglGraphicsWindow.I eglGraphicsWindow.h

#end lib_target
