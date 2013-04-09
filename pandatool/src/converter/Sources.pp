#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3pipeline:c p3event:c p3display:c p3pgraph:c panda:m \
  p3mathutil:c p3linmath:c p3putil:c p3express:c p3pandabase:c \
  p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
  p3dtoolutil:c p3dtoolbase:c p3dtool:m

#begin ss_lib_target
  #define TARGET p3converter
  #define LOCAL_LIBS p3pandatoolbase
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    somethingToEggConverter.I somethingToEggConverter.cxx \
    somethingToEggConverter.h \
    eggToSomethingConverter.I eggToSomethingConverter.cxx \
    eggToSomethingConverter.h

  #define INSTALL_HEADERS \
    somethingToEggConverter.I somethingToEggConverter.h \
    eggToSomethingConverter.I eggToSomethingConverter.h

#end ss_lib_target
