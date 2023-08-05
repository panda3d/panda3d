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

set_target_properties(p3vrpn PROPERTIES CXX_EXCEPTIONS ON)

if(HAVE_VRPN AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  target_link_options(p3vrpn PRIVATE "LINKER:--exclude-libs,libvrpn.a")
  target_link_options(p3vrpn PRIVATE "LINKER:--exclude-libs,libquat.a")
endif()

install(TARGETS p3vrpn
  EXPORT VRPN COMPONENT VRPN
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT VRPNDevel)
install(FILES ${P3VRPN_HEADERS} COMPONENT VRPNDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(VRPN COMPONENT VRPNDevel)
