if(WANT_NATIVE_NET)
  set(P3NATIVENET_HEADERS
      buffered_datagramconnection.h
      buffered_datagramreader.h buffered_datagramreader.I
      ringbuffer.h ringbuffer.I socket_ip.h
      socket_tcp_listen.h time_accumulator.h time_out.h
      socket_address.I socket_address.h
      socket_portable.h  time_base.h time_span.h buffered_datagramwriter.h
      socket_base.h socket_selector.h
      socket_udp.h
      socket_udp_incoming.h time_clock.h
      membuffer.h membuffer.I socket_fdset.h socket_tcp.h
      socket_udp_outgoing.h time_general.h)

  set(P3NATIVENET_SOURCES
      config_nativenet.cxx
      buffered_datagramconnection.cxx
      socket_address.cxx
      socket_ip.cxx
      socket_tcp.cxx
      socket_tcp_listen.cxx
      socket_tcp_ssl.cxx
      socket_udp.cxx
      socket_udp_incoming.cxx
      socket_udp_outgoing.cxx)

  composite_sources(p3nativenet P3NATIVENET_SOURCES)
  add_library(p3nativenet ${P3NATIVENET_HEADERS} ${P3NATIVENET_SOURCES})
  target_link_libraries(p3nativenet p3express p3pandabase p3downloader)
  target_interrogate(p3nativenet ALL)

  install(TARGETS p3nativenet DESTINATION lib)
  install(FILES ${P3NATIVENET_HEADERS} DESTINATION include/panda3d)
endif()
