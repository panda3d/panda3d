#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET switchnode
  #define LOCAL_LIBS \
    display sgraph graph sgraphutil sgattrib gobj putil gsgbase linmath \
    mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
    LODNode.I LODNode.h \
    config_switchnode.h sequenceNode.h \
    switchNodeOne.I switchNodeOne.h
    
  #define INCLUDED_SOURCES \
    LODNode.cxx config_switchnode.cxx \
    sequenceNode.cxx switchNodeOne.cxx  

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

