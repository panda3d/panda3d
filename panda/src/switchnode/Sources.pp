#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET switchnode
  #define LOCAL_LIBS \
    display sgraph graph sgraphutil sgattrib gobj putil gsgbase linmath \
    mathutil

  #define SOURCES \
    LODNode.I LODNode.cxx LODNode.h config_switchnode.cxx \
    config_switchnode.h sequenceNode.cxx sequenceNode.h \
    switchNodeOne.I switchNodeOne.cxx switchNodeOne.h

  #define INSTALL_HEADERS \
    config_switchnode.h LODNode.I LODNode.h sequenceNode.h \
    switchNodeOne.I switchNodeOne.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_sequences
  #define LOCAL_LIBS switchnode

  #define SOURCES \
    test_sequences.cxx

#end test_bin_target

