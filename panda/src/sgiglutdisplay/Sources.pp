#define DIRECTORY_IF_SGIGL yes
#define DIRECTORY_IF_GLUT yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

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

