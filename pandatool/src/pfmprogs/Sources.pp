#define OTHER_LIBS \
    egg:c pandaegg:m \
    pipeline:c event:c display:c gobj:c pgraph:c pstatclient:c panda:m \
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
    pfmTrans.cxx pfmTrans.h

#end bin_target

#begin bin_target
  #define TARGET pfm-bba
  #define LOCAL_LIBS progbase

  #define SOURCES \
    config_pfm.cxx config_pfm.h \
    pfmBba.cxx pfmBba.h

#end bin_target

//#begin bin_target
//  #define TARGET pfm-wallpaper
//  #define LOCAL_LIBS progbase
//
//  #define SOURCES \
//    config_pfm.cxx config_pfm.h \
//    pfmWallpaper.cxx pfmWallpaper.h
//
//#end bin_target
