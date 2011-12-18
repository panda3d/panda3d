#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define LOCAL_LIBS \
    p3pgraph p3event p3gsgbase p3gobj p3putil p3linmath \
    p3downloader p3express p3pandabase p3pstatclient 
 

#begin lib_target
  #define TARGET p3cull

  #define SOURCES \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
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
    cullBinStateSorted.cxx \
    cullBinUnsorted.cxx \
    drawCullHandler.cxx

  #define INSTALL_HEADERS \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    drawCullHandler.h drawCullHandler.I

  #define IGATESCAN all

#end lib_target
