#begin lib_target
  #define TARGET showbase
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    linmath:c \
    event:c \
    putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define SOURCES \
    showBase.cxx showBase.h mersenne.cxx mersenne.h

  #define IGATESCAN all
#end lib_target

#if $[CTPROJS]
  #define INSTALL_SCRIPTS ppython
#endif

