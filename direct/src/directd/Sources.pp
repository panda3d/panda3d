
// This package presently only builds on Windows.
#define DIRECTORY_IF_WINDOWS yes

//#define LOCAL_LIBS \
//    directbase
#define OTHER_LIBS \
    net:c linmath:c putil:c express:c panda:m pandaexpress:m dtoolconfig dtool
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib //advapi32.lib

//#define C++FLAGS -DWITHIN_PANDA

#begin bin_target
  #define TARGET directd

  #define SOURCES \
    directd.h directd.cxx

#end bin_target
