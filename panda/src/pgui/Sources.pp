#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pgui
  #define LOCAL_LIBS \
    audio grutil text tform graph linmath event putil gobj \
    mathutil sgraph sgraphutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    config_pgui.h \
    pgButton.I pgButton.h \
    qppgButton.I qppgButton.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    qppgEntry.I qppgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    qppgItem.I qppgItem.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgTop.I pgTop.h \
    qppgTop.I qppgTop.h \
    pgWaitBar.I pgWaitBar.h \
    qppgWaitBar.I qppgWaitBar.h
    
  #define INCLUDED_SOURCES  \
    config_pgui.cxx \
    pgButton.cxx \
    qppgButton.cxx \
    pgCullTraverser.cxx \
    pgEntry.cxx \
    qppgEntry.cxx \
    pgMouseWatcherGroup.cxx \
    pgMouseWatcherParameter.cxx \
    pgFrameStyle.cxx \
    pgItem.cxx \
    qppgItem.cxx \
    pgMouseWatcherBackground.cxx \
    pgMouseWatcherRegion.cxx \
    pgTop.cxx \
    qppgTop.cxx \
    pgWaitBar.cxx \
    qppgWaitBar.cxx

  #define INSTALL_HEADERS \
    pgButton.I pgButton.h \
    qppgButton.I qppgButton.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    qppgEntry.I qppgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    qppgItem.I qppgItem.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgTop.I pgTop.h \
    qppgTop.I qppgTop.h \
    pgWaitBar.I pgWaitBar.h \
    qppgWaitBar.I qppgWaitBar.h
    

  #define IGATESCAN all

#end lib_target

