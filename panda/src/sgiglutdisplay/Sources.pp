#define BUILD_DIRECTORY $[and $[HAVE_SGIGL],$[HAVE_GLUT]]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgiglutdisplay
  #define LOCAL_LIBS \
    sgidisplay glutdisplay

  #define SOURCES \
    config_sgiglutdisplay.cxx config_sgiglutdisplay.h \
    sgiglutGraphicsPipe.cxx sgiglutGraphicsPipe.h

  #define INSTALL_HEADERS \
    sgiglutGraphicsPipe.h

#end lib_target

