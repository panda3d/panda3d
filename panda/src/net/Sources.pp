#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m
#define DIRECTORY_IF_NET yes
#define USE_NET yes

#begin lib_target
  #define TARGET net
  #define LOCAL_LIBS \
    putil

  #define SOURCES \
    config_net.cxx config_net.h connection.cxx connection.h \
    connectionListener.cxx connectionListener.h connectionManager.N \
    connectionManager.cxx connectionManager.h connectionReader.cxx \
    connectionReader.h connectionWriter.cxx connectionWriter.h \
    datagramQueue.cxx datagramQueue.h datagramTCPHeader.cxx \
    datagramTCPHeader.h datagramUDPHeader.cxx datagramUDPHeader.h \
    netAddress.cxx netAddress.h netDatagram.I netDatagram.cxx \
    netDatagram.h pprerror.cxx pprerror.h queuedConnectionListener.I \
    queuedConnectionListener.cxx queuedConnectionListener.h \
    queuedConnectionManager.cxx queuedConnectionManager.h \
    queuedConnectionReader.cxx queuedConnectionReader.h \
    recentConnectionReader.cxx recentConnectionReader.h

  #define INSTALL_HEADERS \
    config_net.h connection.h connectionListener.h connectionManager.h \
    connectionReader.h connectionWriter.h datagramQueue.h \
    datagramTCPHeader.h datagramUDPHeader.h netAddress.h netDatagram.I \
    netDatagram.h pprerror.h queuedConnectionListener.I \
    queuedConnectionListener.h queuedConnectionManager.h \
    queuedConnectionReader.h queuedReturn.I queuedReturn.h \
    recentConnectionReader.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_datagram
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_datagram.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_spam_client
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_spam_client.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_spam_server
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_spam_server.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_tcp_client
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_tcp_client.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_tcp_server
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_tcp_server.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_udp
  #define LOCAL_LIBS net
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_udp.cxx

#end test_bin_target

