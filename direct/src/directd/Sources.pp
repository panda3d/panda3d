
// This package presently only builds on Windows.
// We also require the network layer (queuedConnectionManager, etc.)
#define BUILD_DIRECTORY $[and $[WINDOWS_PLATFORM],$[HAVE_NET],$[HAVE_DIRECTD]]

#define LOCAL_LIBS \
    directbase
#define OTHER_LIBS \
    $[if $[HAVE_NET],net:c] linmath:c putil:c express:c panda:m pandaexpress:m dtoolconfig dtool
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib //advapi32.lib

#begin lib_target
  #define TARGET directd

  #define SOURCES \
    directd.h directd.cxx
  
  #define INSTALL_HEADERS \
    directd.h

  #define IGATESCAN directd.h

#end lib_target
