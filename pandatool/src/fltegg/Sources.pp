#begin ss_lib_target
  #define TARGET fltegg
  #define LOCAL_LIBS converter flt pandatoolbase
  #define OTHER_LIBS \
    egg:c event:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    fltToEggConverter.I fltToEggConverter.cxx fltToEggConverter.h \
    fltToEggLevelState.I fltToEggLevelState.cxx fltToEggLevelState.h

  #define INSTALL_HEADERS \
    fltToEggConverter.I fltToEggConverter.h \
    fltToEggLevelState.I fltToEggLevelState.h

#end ss_lib_target
