#begin lib_target
  #define TARGET deadrec

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
  correction.h prediction.h  
  
  #define INCLUDED_SOURCES \  
    correction.cxx prediction.cxx

  #define INSTALL_HEADERS \
    correction.h prediction.h

  #define IGATESCAN \
    correction.h prediction.h

  #define LOCAL_LIBS \
    directbase
#end lib_target
