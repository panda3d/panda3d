

#define LOCAL_LIBS express pandabase

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define BUILD_DIRECTORY [$WANT_NATIVE_NET]

#begin lib_target
  #define TARGET nativenet
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
        buffered_datagramconnection.h ringbuffer.h socket_ip.h socket_tcp_listen.h \
        time_accumulator.h time_out.h buffered_datagramreader.h socket_address.h \
        socket_portable.h  time_base.h time_span.h buffered_datagramwriter.h \
        socket_base.h socket_selector.h socket_udp_incoming.h time_clock.h \
        membuffer.h socket_fdset.h socket_tcp.h socket_udp_outgoing.h time_general.h
        
        
  #define INCLUDED_SOURCES \
  
  #define INSTALL_HEADERS \
        buffered_datagramconnection.h ringbuffer.h socket_ip.h socket_tcp_listen.h \
        time_accumulator.h time_out.h buffered_datagramreader.h socket_address.h \
        socket_portable.h time_base.h time_span.h buffered_datagramwriter.h \
        socket_base.h socket_selector.h socket_udp_incoming.h time_clock.h \
        membuffer.h socket_fdset.h socket_tcp.h socket_udp_outgoing.h time_general.h


  #define IGATESCAN all

#end lib_target

