#define OTHER_LIBS \
  egg:c pandaegg:m \
  pipeline:c event:c panda:m \
  mathutil:c linmath:c putil:c express:c pandabase:c \
  interrogatedb:c prc:c dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c dtool:m

#begin ss_lib_target
  #define TARGET converter
  #define LOCAL_LIBS pandatoolbase
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    somethingToEggConverter.I somethingToEggConverter.cxx \
    somethingToEggConverter.h

  #define INSTALL_HEADERS \
    somethingToEggConverter.I somethingToEggConverter.h

#end ss_lib_target
