#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define LOCAL_LIBS \
    pgraph lerp event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient 
 

#begin lib_target
  #define TARGET cull

  #define SOURCES \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinOcclusionTest.h cullBinOcclusionTest.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    drawCullHandler.h drawCullHandler.I

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define INCLUDED_SOURCES \
    binCullHandler.cxx \
    config_cull.cxx \
    cullBinBackToFront.cxx \
    cullBinFixed.cxx \
    cullBinFrontToBack.cxx \
    cullBinOcclusionTest.cxx \
    cullBinStateSorted.cxx \
    cullBinUnsorted.cxx \
    drawCullHandler.cxx

  #define INSTALL_HEADERS \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinOcclusionTest.h cullBinOcclusionTest.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    drawCullHandler.h drawCullHandler.I

  #define IGATESCAN all

#end lib_target
