#define UNIX_SYS_LIBS m

#define OTHER_LIBS \
    p3egg:c p3egg2pg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c] \
    p3pystub

#begin bin_target
  #define TARGET obj2egg
  #define LOCAL_LIBS p3objegg p3eggbase p3progbase

  #define SOURCES \
    objToEgg.cxx objToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2obj
  #define LOCAL_LIBS p3objegg p3eggbase p3progbase

  #define SOURCES \
    eggToObj.cxx eggToObj.h

#end bin_target
