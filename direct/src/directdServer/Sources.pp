
// This package presently only builds on Windows.
#define DIRECTORY_IF_WINDOWS yes

//#define LOCAL_LIBS \
//    directbase
#define OTHER_LIBS \
    net:c linmath:c putil:c express:c panda:m pandaexpress:m dtoolconfig dtool
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib //advapi32.lib

#begin bin_target
  #define TARGET directdServer
  #define LOCAL_LIBS directd
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    directdServer.cxx

#end bin_target
