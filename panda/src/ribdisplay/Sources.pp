#define BUILD_DIRECTORY $[HAVE_RIB]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET ribdisplay
  #define LOCAL_LIBS \
    display ribgsg

  #define SOURCES \
    config_ribdisplay.cxx config_ribdisplay.h ribGraphicsPipe.I \
    ribGraphicsPipe.cxx ribGraphicsPipe.h ribGraphicsWindow.I \
    ribGraphicsWindow.cxx ribGraphicsWindow.h

  #define INSTALL_HEADERS \
    ribGraphicsPipe.I ribGraphicsPipe.h ribGraphicsWindow.I \
    ribGraphicsWindow.h

#end lib_target

