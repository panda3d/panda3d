set(P3TFORM_HEADERS
  buttonThrower.I buttonThrower.h
  config_tform.h
  driveInterface.I driveInterface.h
  mouseInterfaceNode.I mouseInterfaceNode.h
  mouseSubregion.I mouseSubregion.h
  mouseWatcher.h mouseWatcher.I
  mouseWatcherBase.h mouseWatcherBase.I
  mouseWatcherGroup.h
  mouseWatcherParameter.I mouseWatcherParameter.h
  mouseWatcherRegion.I mouseWatcherRegion.h
  trackball.h
  transform2sg.h
)

set(P3TFORM_SOURCES
  buttonThrower.cxx
  config_tform.cxx
  driveInterface.cxx
  mouseInterfaceNode.cxx
  mouseSubregion.cxx
  mouseWatcher.cxx
  mouseWatcherBase.cxx
  mouseWatcherGroup.cxx
  mouseWatcherParameter.cxx
  mouseWatcherRegion.cxx
  trackball.cxx
  transform2sg.cxx
)

composite_sources(p3tform P3TFORM_SOURCES)
add_component_library(p3tform NOINIT SYMBOL BUILDING_PANDA_TFORM
  ${P3TFORM_HEADERS} ${P3TFORM_SOURCES})
target_link_libraries(p3tform p3device p3grutil)
target_interrogate(p3tform ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3tform
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3TFORM_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
