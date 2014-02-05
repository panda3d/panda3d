set(P3TFORM_HEADERS
  buttonThrower.I buttonThrower.h
  config_tform.h
  driveInterface.I driveInterface.h
  mouseInterfaceNode.I mouseInterfaceNode.h
  mouseSubregion.I mouseSubregion.h
  mouseWatcher.I mouseWatcher.h
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
  mouseWatcherGroup.cxx
  mouseWatcherParameter.cxx mouseWatcherRegion.cxx
  trackball.cxx
  transform2sg.cxx
)

composite_sources(p3tform P3TFORM_SOURCES)
add_library(p3tform ${P3TFORM_SOURCES} ${P3TFORM_HEADERS})
target_link_libraries(p3tform p3device p3grutil)
target_interrogate(p3tform ALL)

install(TARGETS p3tform DESTINATION lib)
