#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgmanip
  #define LOCAL_LIBS \
    cull dgraph loader sgraphutil sgattrib sgraph linmath lerp

  #define SOURCES \
    config_sgmanip.cxx config_sgmanip.h findApproxLevel.I \
    findApproxLevel.cxx findApproxLevel.h findApproxPath.I \
    findApproxPath.cxx findApproxPath.h nodePath.I nodePath.cxx \
    nodePath.h nodePathBase.I nodePathBase.cxx nodePathBase.h \
    nodePathCollection.I nodePathCollection.cxx nodePathCollection.h \
    nodePathLerps.cxx nodePathLerps.h

  #define INSTALL_HEADERS \
    nodePath.I nodePath.h nodePathBase.I nodePathBase.h \
    nodePathCollection.I nodePathCollection.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_sgmanip
  #define LOCAL_LIBS \
    sgmanip

  #define SOURCES \
    test_sgmanip.cxx

#end test_bin_target

