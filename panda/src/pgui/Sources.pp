#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pgui
  #define LOCAL_LIBS \
    audio grutil text tform linmath event putil gobj \
    mathutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    config_pgui.h \
    qppgButton.I qppgButton.h \
    pgCullTraverser.I pgCullTraverser.h \
    qppgEntry.I qppgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    qppgItem.I qppgItem.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    qppgTop.I qppgTop.h \
    qppgWaitBar.I qppgWaitBar.h
    
  #define INCLUDED_SOURCES  \
    config_pgui.cxx \
    qppgButton.cxx \
    pgCullTraverser.cxx \
    qppgEntry.cxx \
    pgMouseWatcherGroup.cxx \
    pgMouseWatcherParameter.cxx \
    pgFrameStyle.cxx \
    qppgItem.cxx \
    pgMouseWatcherBackground.cxx \
    pgMouseWatcherRegion.cxx \
    qppgTop.cxx \
    qppgWaitBar.cxx

  #define INSTALL_HEADERS \
    qppgButton.I qppgButton.h \
    pgCullTraverser.I pgCullTraverser.h \
    qppgEntry.I qppgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    qppgItem.I qppgItem.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    qppgTop.I qppgTop.h \
    qppgWaitBar.I qppgWaitBar.h
    

  #define IGATESCAN all

#end lib_target

