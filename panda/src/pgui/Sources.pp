#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3pgui
  #define LOCAL_LIBS \
    p3audio p3grutil p3text p3tform p3linmath p3event p3putil p3gobj \
    p3mathutil

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

  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define LOCAL_LIBS \
    p3framework p3putil p3collide p3pgraph p3chan p3text \
    p3pnmimage p3pnmimagetypes p3event p3gobj p3display \
    p3mathutil p3putil p3express p3dgraph p3device p3tform \
    p3linmath p3pstatclient panda

  #define UNIX_SYS_LIBS m

  #define SOURCES \
    test_pgentry.cxx

#end test_bin_target
