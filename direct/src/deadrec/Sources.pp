#begin lib_target
  #define TARGET deadrec
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    express:c linmath:c \
    interrogatedb:c dconfig:c \
    dtoolutil:c dtoolbase:c dtool:m

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    smoothMover.h smoothMover.I
  
  #define INCLUDED_SOURCES \  
    smoothMover.cxx

  #define INSTALL_HEADERS \
    smoothMover.h smoothMover.I

  #define IGATESCAN \
    all
#end lib_target
