#define DIRECTORY_IF_DX yes

// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDADX

#define COMPONENT_LIBS \
    dxgsg wdxdisplay
#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS dtoolconfig dtool

#begin metalib_target
  #define TARGET pandadx
  #define SOURCES pandadx.cxx  
  #define TARGET_LIBS \
     ddraw.lib d3dim.lib dxguid.lib   \
     winmm.lib kernel32.lib gdi32.lib \
     user32.lib
#end metalib_target
