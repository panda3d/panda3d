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
    pgButton.I pgButton.h \
    pgButtonNotify.I pgButtonNotify.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgItemNotify.I pgItemNotify.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgScrollFrame.I pgScrollFrame.h \
    pgSliderBar.I pgSliderBar.h \
    pgSliderBarNotify.I pgSliderBarNotify.h \
    pgTop.I pgTop.h \
    pgVirtualFrame.I pgVirtualFrame.h \
    pgWaitBar.I pgWaitBar.h
    
  #define INCLUDED_SOURCES  \
    config_pgui.cxx \
    pgButton.cxx \
    pgButtonNotify.cxx \
    pgCullTraverser.cxx \
    pgEntry.cxx \
    pgMouseWatcherGroup.cxx \
    pgMouseWatcherParameter.cxx \
    pgFrameStyle.cxx \
    pgItem.cxx \
    pgItemNotify.cxx \
    pgMouseWatcherBackground.cxx \
    pgMouseWatcherRegion.cxx \
    pgScrollFrame.cxx \
    pgSliderBar.cxx \
    pgSliderBarNotify.cxx \
    pgTop.cxx \
    pgVirtualFrame.cxx \
    pgWaitBar.cxx

  #define INSTALL_HEADERS \
    pgButton.I pgButton.h \
    pgButtonNotify.I pgButtonNotify.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgItemNotify.I pgItemNotify.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgScrollFrame.I pgScrollFrame.h \
    pgSliderBar.I pgSliderBar.h \
    pgSliderBarNotify.I pgSliderBarNotify.h \
    pgTop.I pgTop.h \
    pgVirtualFrame.I pgVirtualFrame.h \
    pgWaitBar.I pgWaitBar.h
    

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_pgentry

  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define LOCAL_LIBS \
    framework putil collide pgraph chan text \
    pnmimage pnmimagetypes event effects gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda

  #define UNIX_SYS_LIBS m

  #define SOURCES \
    test_pgentry.cxx

#end test_bin_target
