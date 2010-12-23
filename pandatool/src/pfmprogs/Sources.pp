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
  #define TARGET pfm-trans
  #define LOCAL_LIBS progbase

  #define SOURCES \
    config_pfm.cxx config_pfm.h \
    pfmFile.cxx pfmFile.h \
    pfmTrans.cxx pfmTrans.h

#end bin_target
