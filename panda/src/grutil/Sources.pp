#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    cardMaker.I cardMaker.h \
    config_grutil.h \
    frameRateMeter.I frameRateMeter.h \
    lineSegs.I lineSegs.h
    
  #define INCLUDED_SOURCES \
    cardMaker.cxx \
    config_grutil.cxx \
    frameRateMeter.cxx \
    lineSegs.cxx

  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    frameRateMeter.I frameRateMeter.h \
    lineSegs.I lineSegs.h

  #define IGATESCAN all

#end lib_target

