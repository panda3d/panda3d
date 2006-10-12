#begin ss_lib_target
  #define TARGET dxfegg
  #define LOCAL_LIBS converter dxf pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pandabase:c express:c pandaexpress:m \
    pipeline:c mathutil:c linmath:c putil:c event:c \
    panda:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    dxfToEggConverter.cxx dxfToEggConverter.h \
    dxfToEggLayer.cxx dxfToEggLayer.h

  #define INSTALL_HEADERS \
    dxfToEggConverter.h dxfToEggLayer.h

#end ss_lib_target
