#define BUILD_DIRECTORY $[HAVE_GLUT]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl glut

#begin lib_target
  #define TARGET glutdisplay
  #define LOCAL_LIBS \
    glgsg display putil gobj gsgbase pnmimage mathutil

  #define SOURCES \
    config_glutdisplay.cxx config_glutdisplay.h glutGraphicsPipe.cxx \
    glutGraphicsPipe.h glutGraphicsWindow.cxx glutGraphicsWindow.h

  #define INSTALL_HEADERS \
    glutGraphicsPipe.h glutGraphicsWindow.h

#end lib_target
