#define DIRECTORY_IF_SGIGL yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET sgidisplay
  #define UNIX_SYS_LIBS \
    Xsgivc

  #define SOURCES \
    config_sgidisplay.cxx config_sgidisplay.h sgiGraphicsPipe.cxx \
    sgiGraphicsPipe.h sgiHardwareChannel.cxx sgiHardwareChannel.h

  #define INSTALL_HEADERS \
    sgiGraphicsPipe.h sgiHardwareChannel.h

#end lib_target

