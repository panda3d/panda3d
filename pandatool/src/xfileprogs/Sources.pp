#define OTHER_LIBS \ 
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS xfileegg xfile eggbase progbase pandatoolbase

  #define SOURCES \
    eggToX.cxx eggToX.h

#end bin_target

#begin bin_target
  #define TARGET x2egg
  #define LOCAL_LIBS xfileegg xfile converter eggbase progbase pandatoolbase

  #define SOURCES \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target

#begin bin_target
  #define TARGET x-trans
  #define LOCAL_LIBS \
    progbase xfile

  #define SOURCES \
    xFileTrans.cxx xFileTrans.h

#end bin_target
