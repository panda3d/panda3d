#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET switchnode
  #define LOCAL_LIBS \
    display sgraph graph sgraphutil sgattrib gobj putil gsgbase linmath \
    mathutil

  #define SOURCES \
    LODNode.I LODNode.cxx LODNode.h config_switchnode.cxx \
    config_switchnode.h sequenceNode.cxx sequenceNode.h

  #define INSTALL_HEADERS \
    LODNode.I LODNode.h sequenceNode.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_sequences
  #define LOCAL_LIBS switchnode

  #define SOURCES \
    test_sequences.cxx

#end test_bin_target

