#begin ss_lib_target
  #define TARGET p3fltegg
  #define LOCAL_LIBS p3converter p3flt p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3mathutil:c p3linmath:c p3putil:c p3event:c p3pipeline:c \
    p3pstatclient:c p3downloader:c p3net:c p3nativenet:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    fltToEggConverter.I fltToEggConverter.cxx fltToEggConverter.h \
    fltToEggLevelState.I fltToEggLevelState.cxx fltToEggLevelState.h

  #define INSTALL_HEADERS \
    fltToEggConverter.I fltToEggConverter.h \
    fltToEggLevelState.I fltToEggLevelState.h

#end ss_lib_target
