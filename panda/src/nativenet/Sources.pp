#define LOCAL_LIBS p3express p3pandabase

#define OTHER_LIBS \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dtool:m

#define BUILD_DIRECTORY $[WANT_NATIVE_NET]
#define USE_PACKAGES native_net openssl

#begin lib_target
  #define TARGET p3nativenet
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    buffered_datagramconnection.h \
    buffered_datagramreader.h buffered_datagramreader.i \
    ringbuffer.h ringbuffer.i socket_ip.h \
    socket_tcp_listen.h time_accumulator.h time_out.h \
    socket_address.I socket_address.h \
    socket_portable.h  time_base.h time_span.h buffered_datagramwriter.h \
    socket_base.h socket_selector.h \
    socket_udp.h \
    socket_udp_incoming.h time_clock.h \
    membuffer.h membuffer.i socket_fdset.h socket_tcp.h \
    socket_udp_outgoing.h time_general.h
    
  #define INCLUDED_SOURCES \
    config_nativenet.cxx \
    buffered_datagramconnection.cxx \
    socket_ip.cxx \
    socket_tcp.cxx \
    socket_tcp_listen.cxx \
    socket_tcp_ssl.cxx \
    socket_udp.cxx \
    socket_udp_incoming.cxx \
    socket_udp_outgoing.cxx
  
  #define INSTALL_HEADERS \
    buffered_datagramconnection.h \
    config_nativenet.h \
    ringbuffer.h ringbuffer.i socket_ip.h socket_tcp_listen.h \
    time_accumulator.h time_out.h \
    buffered_datagramreader.h buffered_datagramreader.i \
    socket_address.I socket_address.h \
    socket_portable.h time_base.h time_span.h buffered_datagramwriter.h \
    socket_base.h socket_selector.h \
    socket_udp.h \
    socket_udp_incoming.h time_clock.h \
    membuffer.h membuffer.i socket_fdset.h socket_tcp.h \
    socket_udp_outgoing.h time_general.h


  #define IGATESCAN all

#end lib_target

