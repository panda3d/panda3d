#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET gsgmisc
  #define LOCAL_LIBS \
    putil gobj gsgbase graph mathutil

  #define SOURCES \
    geomIssuer.I geomIssuer.cxx geomIssuer.h

  #define INSTALL_HEADERS \
    geomIssuer.I geomIssuer.h

#end lib_target

