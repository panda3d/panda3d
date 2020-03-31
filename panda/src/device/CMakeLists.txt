set(P3DEVICE_HEADERS
  analogNode.h analogNode.I
  buttonNode.h buttonNode.I
  clientAnalogDevice.h clientAnalogDevice.I
  clientBase.h clientBase.I
  clientButtonDevice.h clientButtonDevice.I
  clientDevice.h clientDevice.I
  clientDialDevice.h clientDialDevice.I
  clientTrackerDevice.h clientTrackerDevice.I
  config_device.h
  dialNode.h dialNode.I
  evdevInputDevice.h evdevInputDevice.I
  inputDevice.h inputDevice.I
  inputDeviceManager.h inputDeviceManager.I
  inputDeviceNode.h
  inputDeviceSet.h inputDeviceSet.I
  ioKitInputDevice.h
  ioKitInputDeviceManager.h
  linuxInputDeviceManager.h
  linuxJoystickDevice.h linuxJoystickDevice.I
  trackerData.h trackerData.I
  trackerNode.h trackerNode.I
  virtualMouse.h
  winInputDeviceManager.h
  winRawInputDevice.h
  xInputDevice.h
)

set(P3DEVICE_SOURCES
  analogNode.cxx
  buttonNode.cxx
  clientAnalogDevice.cxx
  clientBase.cxx
  clientButtonDevice.cxx
  clientDevice.cxx
  clientDialDevice.cxx
  clientTrackerDevice.cxx
  config_device.cxx
  dialNode.cxx
  evdevInputDevice.cxx
  inputDevice.cxx
  inputDeviceManager.cxx
  inputDeviceNode.cxx
  inputDeviceSet.cxx
  ioKitInputDevice.cxx
  ioKitInputDeviceManager.cxx
  linuxInputDeviceManager.cxx
  linuxJoystickDevice.cxx
  trackerData.cxx
  trackerNode.cxx
  virtualMouse.cxx
  winInputDeviceManager.cxx
  winRawInputDevice.cxx
  xInputDevice.cxx
)

composite_sources(p3device P3DEVICE_SOURCES)
add_component_library(p3device SYMBOL BUILDING_PANDA_DEVICE
  ${P3DEVICE_HEADERS} ${P3DEVICE_SOURCES})
target_link_libraries(p3device p3dgraph)
target_interrogate(p3device ALL)

if(WIN32)
  target_link_libraries(p3device cfgmgr32.lib)

elseif(APPLE)
  find_library(IOKIT_LIBRARY IOKit)
  target_link_libraries(p3device ${IOKIT_LIBRARY})

endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3device
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DEVICE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
