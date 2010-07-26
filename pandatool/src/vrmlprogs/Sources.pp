#define OTHER_LIBS \ 
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

#begin bin_target
  #define TARGET vrml2egg
  #define LOCAL_LIBS pvrml vrmlegg eggbase progbase

  #define SOURCES \
    vrmlToEgg.cxx vrmlToEgg.h

#end bin_target

#begin bin_target
  #define TARGET vrml-trans
  #define LOCAL_LIBS \
    progbase pvrml

  #define SOURCES \
    vrmlTrans.cxx vrmlTrans.h

#end bin_target
