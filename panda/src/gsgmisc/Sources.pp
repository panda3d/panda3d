#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET gsgmisc
  #define LOCAL_LIBS \
    putil gobj gsgbase graph mathutil

  #define SOURCES \
    geomIssuer.I geomIssuer.cxx geomIssuer.h

  #define INSTALL_HEADERS \
    geomIssuer.I geomIssuer.h

#end lib_target

