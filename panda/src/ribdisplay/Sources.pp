#define DIRECTORY_IF_RIB yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET ribdisplay
  #define LOCAL_LIBS \
    display ribgsg sgraph

  #define SOURCES \
    config_ribdisplay.cxx config_ribdisplay.h ribGraphicsPipe.I \
    ribGraphicsPipe.cxx ribGraphicsPipe.h ribGraphicsWindow.I \
    ribGraphicsWindow.cxx ribGraphicsWindow.h

  #define INSTALL_HEADERS \
    ribGraphicsPipe.I ribGraphicsPipe.h ribGraphicsWindow.I \
    ribGraphicsWindow.h

#end lib_target

