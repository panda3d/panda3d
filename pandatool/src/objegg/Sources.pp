#begin ss_lib_target
  #define TARGET p3objegg
  #define LOCAL_LIBS p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c]

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_objegg.cxx config_objegg.h \
    objToEggConverter.cxx objToEggConverter.h objToEggConverter.I \
    eggToObjConverter.cxx eggToObjConverter.h

  #define INSTALL_HEADERS \
    objToEggConverter.h objToEggConverter.I \
    eggToObjConverter.h

#end ss_lib_target
