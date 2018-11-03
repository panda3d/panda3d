if(NOT HAVE_VRPN)
  return()
endif()

set(P3VRPN_HEADERS
  config_vrpn.h
  vrpnAnalogDevice.h vrpnAnalogDevice.I
  vrpnAnalog.h vrpnAnalog.I
  vrpnButtonDevice.h vrpnButtonDevice.I
  vrpnButton.h vrpnButton.I
  vrpnClient.h vrpnClient.I
  vrpnDialDevice.h vrpnDialDevice.I
  vrpnDial.h vrpnDial.I
  vrpn_interface.h
  vrpnTrackerDevice.h vrpnTrackerDevice.I
  vrpnTracker.h vrpnTracker.I
)

set(P3VRPN_SOURCES
  config_vrpn.cxx
  vrpnAnalog.cxx
  vrpnAnalogDevice.cxx
  vrpnButton.cxx
  vrpnButtonDevice.cxx
  vrpnClient.cxx
  vrpnDial.cxx
  vrpnDialDevice.cxx
  vrpnTracker.cxx
  vrpnTrackerDevice.cxx
)

composite_sources(p3vrpn P3VRPN_SOURCES)
add_library(p3vrpn ${P3VRPN_HEADERS} ${P3VRPN_SOURCES})
set_target_properties(p3vrpn PROPERTIES DEFINE_SYMBOL BUILDING_VRPN)
target_link_libraries(p3vrpn panda PKG::VRPN)
target_interrogate(p3vrpn ALL)

install(TARGETS p3vrpn DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${P3VRPN_HEADERS} DESTINATION include/panda3d)
