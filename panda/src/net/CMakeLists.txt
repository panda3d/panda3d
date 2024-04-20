if(NOT HAVE_NET OR NOT WANT_NATIVE_NET)
  return()
endif()

set(P3NET_HEADERS
  config_net.h connection.h connectionListener.h
  connectionManager.N connectionManager.h
  connectionReader.I connectionReader.h
  connectionWriter.h datagramQueue.h
  datagramTCPHeader.I datagramTCPHeader.h
  datagramUDPHeader.I datagramUDPHeader.h
  netAddress.h netDatagram.I netDatagram.h
  datagramGeneratorNet.I datagramGeneratorNet.h
  datagramSinkNet.I datagramSinkNet.h
  queuedConnectionListener.I
  queuedConnectionListener.h queuedConnectionManager.h
  queuedConnectionReader.h recentConnectionReader.h
  queuedReturn.h queuedReturn.I
)

set(P3NET_SOURCES
  config_net.cxx connection.cxx connectionListener.cxx
  connectionManager.cxx connectionReader.cxx
  connectionWriter.cxx datagramQueue.cxx datagramTCPHeader.cxx
  datagramUDPHeader.cxx netAddress.cxx netDatagram.cxx
  datagramGeneratorNet.cxx
  datagramSinkNet.cxx
  queuedConnectionListener.cxx
  queuedConnectionManager.cxx queuedConnectionReader.cxx
  recentConnectionReader.cxx
)

composite_sources(p3net P3NET_SOURCES)
add_component_library(p3net SYMBOL BUILDING_PANDA_NET
  ${P3NET_HEADERS} ${P3NET_SOURCES})
target_link_libraries(p3net p3nativenet p3pipeline p3pandabase pandaexpress)
target_interrogate(p3net ALL)

# These are temporarily built with -module panda3d.net, even though they are
# still part of panda3d.core, see #1466
set_target_properties(p3net PROPERTIES IGATE_MODULE panda3d.net)

if(WIN32)
  target_link_libraries(p3net iphlpapi.lib)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3net
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3NET_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
