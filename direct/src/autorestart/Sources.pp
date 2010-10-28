#begin bin_target
  // This program only compiles on Unix.
  #define BUILD_TARGET $[UNIX_PLATFORM]
  #define C++FLAGS -DWITHIN_PANDA

  #define TARGET autorestart
  #define SOURCES autorestart.c
#end bin_target
