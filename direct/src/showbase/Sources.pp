#begin lib_target
  #define TARGET showbase
  #define LOCAL_LIBS \
    pandatoolbase
  #define OTHER_LIBS \
    linmath:c putil:c express:c panda:m pandaexpress:m dtool

  #define SOURCES \
    showBase.cxx showBase.h

  #define IGATESCAN all
#end lib_target

#define INSTALL_SCRIPTS ppython
