#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define LOCAL_LIBS \
    pgraph lerp event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient 
 

#begin lib_target
  #define TARGET cull

  #define SOURCES \
    binCullHandler.h \
    config_cull.h \
    cullBinBackToFront.h cullBinFixed.h \
    cullBinFrontToBack.h cullBinStateSorted.h cullBinUnsorted.h \
    drawCullHandler.h binCullHandler.I cullBinBackToFront.I \
    cullBinFixed.I cullBinFrontToBack.I cullBinStateSorted.I \
    cullBinUnsorted.I drawCullHandler.I

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define INCLUDED_SOURCES \
    binCullHandler.cxx \
    config_cull.cxx \
    cullBinBackToFront.cxx \
    cullBinFixed.cxx \
    cullBinFrontToBack.cxx \
    cullBinStateSorted.cxx \
    cullBinUnsorted.cxx \
    drawCullHandler.cxx

  #define INSTALL_HEADERS \
    binCullHandler.h \
    config_cull.h \
    cullBinBackToFront.h cullBinFixed.h \
    cullBinFrontToBack.h cullBinStateSorted.h cullBinUnsorted.h \
    drawCullHandler.h binCullHandler.I cullBinBackToFront.I \
    cullBinFixed.I cullBinFrontToBack.I cullBinStateSorted.I \
    cullBinUnsorted.I drawCullHandler.I

  #define IGATESCAN all

#end lib_target
