#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET cull
  #define LOCAL_LIBS \
    gobj sgraphutil graph putil sgraph mathutil sgattrib display \
    pstatclient

  #define SOURCES \
    config_cull.cxx config_cull.h cullState.I cullState.cxx cullState.h \
    cullStateLookup.I cullStateLookup.cxx cullStateLookup.h \
    cullStateSubtree.I cullStateSubtree.cxx cullStateSubtree.h \
    cullTraverser.I cullTraverser.cxx cullTraverser.h \
    directRenderTransition.I directRenderTransition.cxx \
    directRenderTransition.h geomBin.I geomBin.cxx geomBin.h \
    geomBinAttribute.I geomBinAttribute.N geomBinAttribute.cxx \
    geomBinAttribute.h geomBinBackToFront.I geomBinBackToFront.cxx \
    geomBinBackToFront.h geomBinFixed.I geomBinFixed.cxx geomBinFixed.h \
    geomBinGroup.I geomBinGroup.cxx geomBinGroup.h geomBinNormal.cxx \
    geomBinNormal.h geomBinTransition.I geomBinTransition.cxx \
    geomBinTransition.h geomBinUnsorted.I geomBinUnsorted.cxx \
    geomBinUnsorted.h

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

#end lib_target

#begin test_bin_target
  #define TARGET test_cull
  #define LOCAL_LIBS \
    cull sgattrib

  #define SOURCES \
    test_cull.cxx

#end test_bin_target

