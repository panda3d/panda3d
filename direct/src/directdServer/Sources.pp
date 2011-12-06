
// This package presently only builds on Windows.
// We also require the network layer (queuedConnectionManager, etc.)
#define BUILD_DIRECTORY $[and $[WINDOWS_PLATFORM],$[HAVE_NET],$[HAVE_DIRECTD]]

//#define LOCAL_LIBS \
//    p3directbase
#define OTHER_LIBS \
    $[if $[HAVE_NET],p3net:c] p3linmath:c p3putil:c p3express:c panda:m pandaexpress:m p3dtoolconfig p3dtool
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib //advapi32.lib

#begin bin_target
  #define TARGET p3directdServer
  #define LOCAL_LIBS p3directd
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    directdServer.cxx directdServer.h

#end bin_target

#begin test_bin_target
  #define TARGET directdClient
  #define LOCAL_LIBS p3directd
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    directdClient.cxx directdClient.h

#end test_bin_target
