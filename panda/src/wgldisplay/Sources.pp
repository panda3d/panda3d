#define DIRECTORY_IF_WGL yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET wgldisplay
  #define LOCAL_LIBS \
    glgsg display putil

  #define SOURCES \
    config_wgldisplay.cxx config_wgldisplay.h wglGraphicsPipe.cxx \
    wglGraphicsPipe.h wglGraphicsWindow.cxx wglGraphicsWindow.h

  #define INSTALL_HEADERS \
    wglGraphicsPipe.h wglGraphicsWindow.h

#end lib_target

#begin test_bin_target
  #define TARGET test_wgl
  #define LOCAL_LIBS \
    putil graph display mathutil gobj sgraph wgldisplay glgsg

  #define SOURCES \
    test_wgl.cxx

#end test_bin_target

