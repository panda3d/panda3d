#begin ss_lib_target
  #define TARGET converter
  #define LOCAL_LIBS pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    somethingToEggConverter.I somethingToEggConverter.cxx \
    somethingToEggConverter.h

  #define INSTALL_HEADERS \
    somethingToEggConverter.I somethingToEggConverter.h

#end ss_lib_target
