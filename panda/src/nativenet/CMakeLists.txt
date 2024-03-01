if(NOT WANT_NATIVE_NET)
  return()
endif()

set(P3NATIVENET_HEADERS
  config_nativenet.h
  buffered_datagramconnection.h
  buffered_datagramreader.h buffered_datagramreader.I
  buffered_datagramwriter.h buffered_datagramwriter.I
  ringbuffer.h ringbuffer.I socket_ip.h
  socket_tcp.h socket_tcp_listen.h
  time_accumulator.h time_out.h
  socket_address.I socket_address.h
  socket_portable.h time_base.h time_span.h
  socket_base.h socket_selector.h
  socket_udp.h
  socket_udp_incoming.h time_clock.h
  membuffer.h membuffer.I socket_fdset.h
  socket_udp_outgoing.h time_general.h
)

set(P3NATIVENET_SOURCES
  config_nativenet.cxx
  buffered_datagramconnection.cxx
  socket_address.cxx
  socket_ip.cxx
  socket_tcp.cxx
  socket_tcp_listen.cxx
  socket_udp.cxx
  socket_udp_incoming.cxx
  socket_udp_outgoing.cxx
)

composite_sources(p3nativenet P3NATIVENET_SOURCES)
add_component_library(p3nativenet SYMBOL BUILDING_PANDA_NATIVENET
  ${P3NATIVENET_HEADERS} ${P3NATIVENET_SOURCES})
target_link_libraries(p3nativenet p3pandabase pandaexpress)
target_interrogate(p3nativenet ALL)

# These are temporarily built with -module panda3d.net, even though they are
# still part of panda3d.core, see #1466
set_target_properties(p3nativenet PROPERTIES IGATE_MODULE panda3d.net)

if(NOT BUILD_METALIBS)
  install(TARGETS p3nativenet
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3NATIVENET_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
