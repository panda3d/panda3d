set(P3DEVICE_HEADERS
  analogNode.I analogNode.h
  buttonNode.I buttonNode.h
  clientAnalogDevice.I clientAnalogDevice.h clientBase.I
  clientBase.h clientButtonDevice.I clientButtonDevice.h
  clientDevice.I clientDevice.h clientDialDevice.I
  clientDialDevice.h clientTrackerDevice.I
  clientTrackerDevice.h config_device.h
  dialNode.I dialNode.h
  mouseAndKeyboard.h
  trackerData.I trackerData.h
  trackerNode.I trackerNode.h
  virtualMouse.h
)
set(P3DEVICE_SOURCES
  analogNode.cxx
  buttonNode.cxx
  clientAnalogDevice.cxx
  clientBase.cxx clientButtonDevice.cxx clientDevice.cxx
  clientDialDevice.cxx clientTrackerDevice.cxx
  config_device.cxx
  dialNode.cxx
  mouseAndKeyboard.cxx
  trackerData.cxx
  trackerNode.cxx
  virtualMouse.cxx
)

composite_sources(p3device P3DEVICE_SOURCES)
add_library(p3device ${P3DEVICE_HEADERS} ${P3DEVICE_SOURCES})
target_link_libraries(p3device p3dgraph p3display)
target_interrogate(p3device ALL)

install(TARGETS p3device DESTINATION lib)
