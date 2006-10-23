#begin lib_target
  #define TARGET deadrec
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    express:c linmath:c \
    interrogatedb:c dconfig:c \
    dtoolutil:c dtoolbase:c dtool:m \
    prc:c pandabase:c putil:c \
    pipeline:c

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_deadrec.h \
    smoothMover.h smoothMover.I
  
  #define INCLUDED_SOURCES \  
    config_deadrec.cxx \
    smoothMover.cxx

  #define INSTALL_HEADERS \
    config_deadrec.h \
    smoothMover.h smoothMover.I

  #define IGATESCAN \
    all
#end lib_target
