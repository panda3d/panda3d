#begin lib_target
  #define TARGET deadrec

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    smoothMover.h smoothMover.I
  
  #define INCLUDED_SOURCES \  
    smoothMover.cxx

  #define INSTALL_HEADERS \
    smoothMover.h smoothMover.I

  #define IGATESCAN \
    all

  #define LOCAL_LIBS \
    directbase
#end lib_target
