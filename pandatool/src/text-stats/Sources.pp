#define BUILD_DIRECTORY $[HAVE_NET]

#begin bin_target
  #define TARGET text-stats
  #define LOCAL_LIBS \
    progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c pipeline:c event:c \
    pnmimage:c mathutil:c \
    $[if $[HAVE_NET],net:c] \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    textMonitor.cxx textMonitor.h textStats.cxx textStats.h

  #define INSTALL_HEADERS 

#end bin_target

