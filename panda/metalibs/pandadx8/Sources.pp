#define DIRECTORY_IF_DX yes

// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDADX

#define COMPONENT_LIBS \
    dxgsg8 wdxdisplay8
#define LOCAL_LIBS gsgbase display express gobj
#define OTHER_LIBS dtoolconfig dtool

#if $[BUILD_DX8]
#begin metalib_target
  #define TARGET pandadx8
  #define SOURCES pandadx8.cxx
  #define TARGET_LIBS d3d8.lib dxerr8.lib
#end metalib_target
#endif
