#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET gsgbase
  #define LOCAL_LIBS \
    putil graph

  #define SOURCES \
    config_gsgbase.cxx config_gsgbase.h graphicsStateGuardianBase.cxx \
    graphicsStateGuardianBase.h

  #define INSTALL_HEADERS \
    graphicsStateGuardianBase.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_gsgbase
  #define LOCAL_LIBS \
    gsgbase

  #define SOURCES \
    test_gsgbase.cxx

#end test_bin_target

