#begin bin_target
  #define TARGET bam-info
  #define LOCAL_LIBS \
    progbase
  #define OTHER_LIBS \
    parametrics:c collide:c chan:c char:c switchnode:c \
    cull:c loader:c egg:c sgraphutil:c sgattrib:c \
    sgraph:c pnmimagetypes:c \
    graph:c putil:c linmath:c express:c panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    bamInfo.cxx bamInfo.h

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET egg2bam
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    sgmanip:c cull:c sgattrib:c sgraph:c dgraph:c \
    loader:c egg2sg:c builder:c egg:c pnmimagetypes:c gobj:c \
    chan:c graph:c putil:c linmath:c \
    express:c pandaegg:m panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    eggToBam.cxx eggToBam.h

#end bin_target


#begin bin_target
  #define TARGET bam2egg
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    parametrics:c collide:c chan:c char:c switchnode:c \
    cull:c loader:c egg:c sgraphutil:c sgattrib:c \
    sgraph:c pnmimagetypes:c \
    graph:c putil:c linmath:c express:c panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    bamToEgg.cxx bamToEgg.h

#end bin_target

