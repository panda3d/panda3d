#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET cull
  #define LOCAL_LIBS \
    gobj sgraphutil graph putil sgraph mathutil sgattrib display \
    pstatclient
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     config_cull.h cullState.I cullState.h cullStateLookup.I  \
     cullStateLookup.h cullStateSubtree.I cullStateSubtree.h  \
     cullTraverser.I cullTraverser.h directRenderTransition.I  \
     directRenderTransition.h geomBin.I geomBin.h  \
     geomBinAttribute.I geomBinAttribute.N geomBinAttribute.h  \
     geomBinBackToFront.I geomBinBackToFront.h geomBinFixed.I  \
     geomBinFixed.h geomBinGroup.I geomBinGroup.h geomBinNormal.h  \
     geomBinTransition.I geomBinTransition.h geomBinUnsorted.I  \
     geomBinUnsorted.h 
    
  #define INCLUDED_SOURCES \
     config_cull.cxx cullState.cxx cullStateLookup.cxx  \
     cullStateSubtree.cxx cullTraverser.cxx  \
     directRenderTransition.cxx geomBin.cxx geomBinAttribute.cxx  \
     geomBinBackToFront.cxx geomBinFixed.cxx geomBinGroup.cxx  \
     geomBinNormal.cxx geomBinTransition.cxx geomBinUnsorted.cxx 

  #define INSTALL_HEADERS \
    config_cull.h cullLevelState.h cullState.I cullState.h \
    cullStateLookup.I cullStateLookup.h cullStateSubtree.I \
    cullStateSubtree.h cullTraverser.I cullTraverser.h \
    directRenderTransition.I directRenderTransition.h geomBin.I \
    geomBin.h geomBinAttribute.I geomBinAttribute.h \
    geomBinBackToFront.I geomBinBackToFront.h geomBinFixed.I \
    geomBinFixed.h geomBinGroup.I geomBinGroup.h geomBinNormal.h \
    geomBinTransition.I geomBinTransition.h geomBinUnsorted.I \
    geomBinUnsorted.h

  #define IGATESCAN all
  
//  #define PRECOMPILED_HEADER cull_headers.h

#end lib_target

#begin test_bin_target
  #define TARGET test_cull
  #define LOCAL_LIBS \
    cull sgattrib

  #define SOURCES \
    test_cull.cxx

#end test_bin_target

