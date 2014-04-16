set(P3PGUI_HEADERS
  config_pgui.h
  pgButton.I pgButton.h
  pgButtonNotify.I pgButtonNotify.h
  pgCullTraverser.I pgCullTraverser.h
  pgEntry.I pgEntry.h
  pgMouseWatcherGroup.I pgMouseWatcherGroup.h
  pgMouseWatcherParameter.I pgMouseWatcherParameter.h
  pgFrameStyle.I pgFrameStyle.h
  pgItem.I pgItem.h
  pgItemNotify.I pgItemNotify.h
  pgMouseWatcherBackground.h
  pgMouseWatcherRegion.I pgMouseWatcherRegion.h
  pgScrollFrame.I pgScrollFrame.h
  pgSliderBar.I pgSliderBar.h
  pgSliderBarNotify.I pgSliderBarNotify.h
  pgTop.I pgTop.h
  pgVirtualFrame.I pgVirtualFrame.h
  pgWaitBar.I pgWaitBar.h
)
set(P3PGUI_SOURCES
  config_pgui.cxx
  pgButton.cxx
  pgButtonNotify.cxx
  pgCullTraverser.cxx
  pgEntry.cxx
  pgMouseWatcherGroup.cxx
  pgMouseWatcherParameter.cxx
  pgFrameStyle.cxx
  pgItem.cxx
  pgItemNotify.cxx
  pgMouseWatcherBackground.cxx
  pgMouseWatcherRegion.cxx
  pgScrollFrame.cxx
  pgSliderBar.cxx
  pgSliderBarNotify.cxx
  pgTop.cxx
  pgVirtualFrame.cxx
  pgWaitBar.cxx
)

composite_sources(p3pgui P3PGUI_SOURCES)
add_library(p3pgui ${P3PGUI_HEADERS} ${P3PGUI_SOURCES})
target_link_libraries(p3pgui p3tform)
target_interrogate(p3pgui ALL)

install(TARGETS p3pgui DESTINATION lib)

#begin test_bin_target
  #define TARGET test_pgentry

  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define LOCAL_LIBS \
#    p3framework p3putil p3collide p3pgraph p3chan p3text \
#    p3pnmimage p3pnmimagetypes p3event p3gobj p3display \
#    p3mathutil p3putil p3express p3dgraph p3device p3tform \
#    p3linmath p3pstatclient panda

  #define UNIX_SYS_LIBS m

  #define SOURCES \
#    test_pgentry.cxx

#end test_bin_target
