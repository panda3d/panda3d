#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

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


#if $[HAVE_DX]
  #begin static_lib_target
    #define TARGET framework_multimon
    #define LOCAL_LIBS \
      putil collide loader sgmanip chan text chancfg cull \
      pnmimage pnmimagetypes event wdxdisplay

    #define SOURCES \
      config_framework.cxx config_framework.h framework_multimon.cxx framework.h

    #define INSTALL_HEADERS \
      framework.h

  #end static_lib_target
#endif


