#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pgui
  #define LOCAL_LIBS \
    text tform graph linmath event putil gobj \
    mathutil sgraph sgraphutil

//  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    config_pgui.h \
    pgButton.I pgButton.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgTop.I pgTop.h
     
//  #define INCLUDED_SOURCES  \
  #define SOURCES $[SOURCES] \
    config_pgui.cxx \
    pgButton.cxx \
    pgMouseWatcherGroup.cxx \
    pgMouseWatcherParameter.cxx \
    pgFrameStyle.cxx \
    pgItem.cxx \
    pgMouseWatcherRegion.cxx \
    pgTop.cxx

  #define INSTALL_HEADERS \
    pgButton.I pgButton.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgTop.I pgTop.h
    

  #define IGATESCAN all

#end lib_target

