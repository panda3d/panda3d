#define BUILD_DIRECTORY $[HAVE_SGIGL]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgidisplay
  #define UNIX_SYS_LIBS \
    Xsgivc
  #define LOCAL_LIBS display

  #define SOURCES \
    config_sgidisplay.cxx config_sgidisplay.h sgiGraphicsPipe.cxx \
    sgiGraphicsPipe.h sgiHardwareChannel.cxx sgiHardwareChannel.h

  #define INSTALL_HEADERS \
    sgiGraphicsPipe.h sgiHardwareChannel.h

#end lib_target

