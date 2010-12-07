#begin ss_lib_target
  #define TARGET objegg
  #define LOCAL_LIBS converter pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_objegg.cxx config_objegg.h \
    objToEggConverter.cxx objToEggConverter.h

  #define INSTALL_HEADERS \
    objToEggConverter.h

#end ss_lib_target
