#define DIRECTORY_IF_WGL yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET wgldisplay
  #define LOCAL_LIBS \
    glgsg display putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_wgldisplay.h wglGraphicsPipe.h wglGraphicsWindow.cxx wglGraphicsWindow.h wglext.h
    
  #define INCLUDED_SOURCES \
    config_wgldisplay.cxx wglGraphicsPipe.cxx

  #define INSTALL_HEADERS \
     config_wgldisplay.h wglGraphicsPipe.h wglGraphicsWindow.h

#end lib_target

#begin test_bin_target
  #define TARGET test_wgl
  #define LOCAL_LIBS \
    putil graph display mathutil gobj sgraph wgldisplay glgsg

  #define SOURCES \
    test_wgl.cxx

#end test_bin_target

