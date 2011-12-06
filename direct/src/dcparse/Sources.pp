#define LOCAL_LIBS \
  p3dcparser
#define OTHER_LIBS \
  p3express:c pandaexpress:m \
  p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
  p3dtoolutil:c p3dtoolbase:c p3dtool:m \
  p3prc:c p3pstatclient:c p3pandabase:c p3linmath:c p3putil:c \
  p3pipeline:c p3downloader:c \
  $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
  panda:m \
  p3pystub

#define C++FLAGS -DWITHIN_PANDA

#begin bin_target
  #define TARGET dcparse
  #define USE_PACKAGES zlib openssl tar

  #define SOURCES \
    dcparse.cxx
  #define WIN_SYS_LIBS shell32.lib
#end bin_target

