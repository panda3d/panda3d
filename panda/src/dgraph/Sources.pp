#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET dgraph
  #define LOCAL_LIBS \
    pstatclient pgraph putil mathutil event

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx  
  
  #define SOURCES \
    config_dgraph.h \
    qpdataGraphTraverser.I qpdataGraphTraverser.h \
    qpdataNode.I qpdataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h
    
 #define INCLUDED_SOURCES \
    config_dgraph.cxx \
    qpdataGraphTraverser.cxx \
    qpdataNode.cxx \
    dataNodeTransmit.cxx

  #define INSTALL_HEADERS \
    config_dgraph.h \
    qpdataGraphTraverser.I qpdataGraphTraverser.h \
    qpdataNode.I qpdataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h

  #define IGATESCAN \
    all

#end lib_target
