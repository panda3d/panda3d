#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    sgraph gobj linmath putil

  #define SOURCES \
    config_grutil.cxx config_grutil.h lineSegs.I lineSegs.cxx \
    lineSegs.h

  #define INSTALL_HEADERS \
    lineSegs.I lineSegs.h

  #define IGATESCAN all

#end lib_target

