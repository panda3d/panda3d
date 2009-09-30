#define BUILD_DIRECTORY $[HAVE_EGL]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET egldisplay
  #define BUILD_TARGET $[HAVE_GLES]
  #define USE_PACKAGES gles egl x11
  #define EXTRA_CDEFS OPENGLES_1
  #define LOCAL_LIBS \
    glesgsg x11display

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
    gles2gsg

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
