#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin static_lib_target
  #define TARGET framework
  #define LOCAL_LIBS \
    putil collide loader sgmanip chan text chancfg cull \
    pnmimage pnmimagetypes event

  #define SOURCES \
    config_framework.cxx config_framework.h framework.cxx framework.h

  #define INSTALL_HEADERS \
    framework.h

#end static_lib_target

