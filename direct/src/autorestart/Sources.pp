#begin bin_target
  // This program only compiles on Unix.
  #define BUILD_TARGET $[UNIX_PLATFORM]

  #define TARGET autorestart
  #define SOURCES autorestart.c
#end bin_target
