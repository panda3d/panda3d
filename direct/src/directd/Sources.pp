
// This package presently only builds on Windows.
#define DIRECTORY_IF_WINDOWS yes

// We also require the network layer (queuedConnectionManager, etc.)
#define DIRECTORY_IF_NET yes

#define LOCAL_LIBS \
    directbase
#define OTHER_LIBS \
    net:c linmath:c putil:c express:c panda:m pandaexpress:m dtoolconfig dtool
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib //advapi32.lib

#begin lib_target
  #define TARGET directd

  #define SOURCES \
    directd.h directd.cxx
  
  #define INSTALL_HEADERS \
    directd.h

  #define IGATESCAN directd.h

#end lib_target
