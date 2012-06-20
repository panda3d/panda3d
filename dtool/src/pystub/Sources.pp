//#define BUILDING_DLL BUILDING_PYSTUB
#define LOCAL_LIBS p3dtoolbase

#begin static_lib_target
  #define TARGET p3pystub
  
  #define SOURCES \
    pystub.cxx pystub.h

  #define INSTALL_HEADERS \
    pystub.h

#end static_lib_target
