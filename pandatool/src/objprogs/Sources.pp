#define UNIX_SYS_LIBS m

#define OTHER_LIBS \
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    pandaexpress:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c] \
    pystub

#begin bin_target
  #define TARGET obj2egg
  #define LOCAL_LIBS objegg eggbase progbase

  #define SOURCES \
    objToEgg.cxx objToEgg.h

#end bin_target
