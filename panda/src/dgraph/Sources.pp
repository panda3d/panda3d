#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3dgraph
  #define LOCAL_LIBS \
    p3pstatclient p3pgraph p3putil p3mathutil p3event

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx  
  
  #define SOURCES \
    config_dgraph.h \
    dataGraphTraverser.I dataGraphTraverser.h \
    dataNode.I dataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h
    
 #define INCLUDED_SOURCES \
    config_dgraph.cxx \
    dataGraphTraverser.cxx \
    dataNode.cxx \
    dataNodeTransmit.cxx

  #define INSTALL_HEADERS \
    config_dgraph.h \
    dataGraphTraverser.I dataGraphTraverser.h \
    dataNode.I dataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h

  #define IGATESCAN \
    all

#end lib_target
