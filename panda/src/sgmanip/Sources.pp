#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgmanip
  #define LOCAL_LIBS \
    cull dgraph loader sgraphutil sgattrib sgraph linmath lerp

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
     config_sgmanip.h findApproxLevel.I findApproxLevel.h  \
     findApproxPath.I findApproxPath.h nodePath.I nodePath.h  \
     nodePathBase.I nodePathBase.h nodePathCollection.I  \
     nodePathCollection.h nodePathLerps.h nodePath.cxx
    
  #define INCLUDED_SOURCES \
     config_sgmanip.cxx findApproxLevel.cxx findApproxPath.cxx  \
     nodePathBase.cxx nodePathCollection.cxx nodePathLerps.cxx 

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

