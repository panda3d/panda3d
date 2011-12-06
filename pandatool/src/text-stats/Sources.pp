#define BUILD_DIRECTORY $[HAVE_NET]

#begin bin_target
  #define TARGET text-stats
  #define LOCAL_LIBS \
    p3progbase p3pstatserver
  #define OTHER_LIBS \
    p3pstatclient:c p3linmath:c p3putil:c p3pipeline:c p3event:c \
    p3pnmimage:c p3mathutil:c \
    p3downloader:c $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub

  #define SOURCES \
    textMonitor.cxx textMonitor.h textMonitor.I \
    textStats.cxx textStats.h

  #define INSTALL_HEADERS 

#end bin_target

