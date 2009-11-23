#define LOCAL_LIBS \
  dcparser
#define OTHER_LIBS \
  express:c pandaexpress:m \
  interrogatedb:c dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c dtool:m \
  prc:c pstatclient:c pandabase:c linmath:c putil:c \
  pipeline:c downloader:c \
  $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
  panda:m \
  pystub

#define C++FLAGS -DWITHIN_PANDA

#begin bin_target
  #define TARGET dcparse
  #define USE_PACKAGES zlib openssl tar

  #define SOURCES \
    dcparse.cxx
  #define WIN_SYS_LIBS shell32.lib
#end bin_target

