#begin ss_lib_target
  #define TARGET dxfegg
  #define LOCAL_LIBS converter dxf pandatoolbase
  #define OTHER_LIBS \
    egg:c event:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    dxfToEggConverter.cxx dxfToEggConverter.h \
    dxfToEggLayer.cxx dxfToEggLayer.h

  #define INSTALL_HEADERS \
    dxfToEggConverter.h dxfToEggLayer.h

#end ss_lib_target
