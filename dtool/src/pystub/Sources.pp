#define BUILDING_DLL BUILDING_DTOOL
#define LOCAL_LIBS dtoolbase

#begin lib_target
  #define TARGET pystub
  
  #define SOURCES \
    pystub.cxx pystub.h

  #define INSTALL_HEADERS \
    pystub.h

#end lib_target
