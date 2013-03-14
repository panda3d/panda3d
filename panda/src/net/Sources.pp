#define OTHER_LIBS \
   p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
   p3dtoolutil:c p3dtoolbase:c p3prc:c p3dtool:m
#define WIN_SYS_LIBS Iphlpapi.lib

#define BUILD_DIRECTORY $[and $[HAVE_NET],$[WANT_NATIVE_NET]]
#define USE_PACKAGES net

#begin lib_target
  #define TARGET p3net
  #define LOCAL_LIBS \
    p3express p3downloader p3pandabase p3nativenet p3pipeline

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
     config_net.h connection.h connectionListener.h  \
     connectionManager.N connectionManager.h \
     connectionReader.I connectionReader.h  \
     connectionWriter.h datagramQueue.h \
     datagramTCPHeader.I datagramTCPHeader.h  \
     datagramUDPHeader.I datagramUDPHeader.h  \
     netAddress.h netDatagram.I netDatagram.h  \
     datagramGeneratorNet.I datagramGeneratorNet.h \
     datagramSinkNet.I datagramSinkNet.h \
     queuedConnectionListener.I  \
     queuedConnectionListener.h queuedConnectionManager.h  \
     queuedConnectionReader.h recentConnectionReader.h \
     queuedReturn.h queuedReturn.I

  #define INCLUDED_SOURCES \
     config_net.cxx connection.cxx connectionListener.cxx  \
     connectionManager.cxx connectionReader.cxx  \
     connectionWriter.cxx datagramQueue.cxx datagramTCPHeader.cxx  \
     datagramUDPHeader.cxx netAddress.cxx netDatagram.cxx  \
     datagramGeneratorNet.cxx \
     datagramSinkNet.cxx \
     queuedConnectionListener.cxx  \
     queuedConnectionManager.cxx queuedConnectionReader.cxx  \
     recentConnectionReader.cxx 

  #define INSTALL_HEADERS \
    config_net.h connection.h connectionListener.h connectionManager.h \
    connectionReader.I connectionReader.h  \
    connectionWriter.h datagramQueue.h \
    datagramTCPHeader.I datagramTCPHeader.h \
    datagramUDPHeader.I datagramUDPHeader.h \
    netAddress.h netDatagram.I \
    netDatagram.h queuedConnectionListener.I \
    datagramGeneratorNet.I datagramGeneratorNet.h \
    datagramSinkNet.I datagramSinkNet.h \
    queuedConnectionListener.h queuedConnectionManager.h \
    queuedConnectionReader.h queuedReturn.I queuedReturn.h \
    recentConnectionReader.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_datagram
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    test_datagram.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_spam_client
  #define LOCAL_LIBS p3net p3putil
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_spam_client.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_spam_server
  #define LOCAL_LIBS p3net p3putil
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_spam_server.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_tcp_client
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_tcp_client.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_tcp_server
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_tcp_server.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_raw_server
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    test_raw_server.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_udp
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    datagram_ui.cxx datagram_ui.h test_udp.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET fake_http_server
  #define LOCAL_LIBS p3net
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    fake_http_server.cxx

#end test_bin_target
