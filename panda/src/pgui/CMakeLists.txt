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
add_component_library(p3pgui SYMBOL BUILDING_PANDA_PGUI
  ${P3PGUI_HEADERS} ${P3PGUI_SOURCES})
target_link_libraries(p3pgui p3tform)
target_interrogate(p3pgui ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3pgui
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PGUI_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
