// This directory is still experimental.  Define HAVE_P3D_PLUGIN in
// your Config.pp to build it.
#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL],$[HAVE_ZLIB]]

#begin bin_target
  #define USE_PACKAGES openssl zlib
  #define TARGET panda3d

  #define LOCAL_LIBS plugin_common

  #define OTHER_LIBS \
    prc:c dtoolutil:c dtoolbase:c dtool:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    express:c downloader:c pandaexpress:m \
    pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c panda:m \
    pystub

  #define OSX_SYS_FRAMEWORKS Foundation AppKit

  #define SOURCES \
    panda3d.cxx

  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib

#end bin_target
