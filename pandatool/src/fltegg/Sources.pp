#begin ss_lib_target
  #define TARGET fltegg
  #define LOCAL_LIBS pandatoolbase progbase flt
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m dtoolconfig dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    fltToEggConverter.I fltToEggConverter.cxx fltToEggConverter.h

  #define INSTALL_HEADERS \
    fltToEggConverter.I fltToEggConverter.h

#end ss_lib_target
