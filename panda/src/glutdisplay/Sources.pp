#define DIRECTORY_IF_GL yes
#define DIRECTORY_IF_GLUT yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase
#define USE_GL yes

#begin lib_target
  #define TARGET glutdisplay
  #define LOCAL_LIBS \
    glgsg display putil gobj gsgbase pnmimage mathutil graph sgraph \
    light

  #define SOURCES \
    config_glutdisplay.cxx config_glutdisplay.h glutGraphicsPipe.cxx \
    glutGraphicsPipe.h glutGraphicsWindow.cxx glutGraphicsWindow.h

  #define INSTALL_HEADERS \
    glutGraphicsPipe.h glutGraphicsWindow.h

#end lib_target

#begin test_bin_target
  #define TARGET test_glut
  #define LOCAL_LIBS \
    putil graph display mathutil gobj sgraph

  #define SOURCES \
    test_glut.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_glut_win

  #define SOURCES \
    test_glut_win.cxx

#end test_bin_target

