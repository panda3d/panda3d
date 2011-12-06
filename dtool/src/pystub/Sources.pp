#define BUILDING_DLL BUILDING_DTOOLCONFIG
#define LOCAL_LIBS p3dtoolbase

#begin lib_target
  #define TARGET p3pystub
  
  #define SOURCES \
    pystub.cxx pystub.h

  #define INSTALL_HEADERS \
    pystub.h

#end lib_target
