// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAEXPRESS
#define USE_PACKAGES p3net

#define COMPONENT_LIBS p3downloader p3express p3pandabase
#define OTHER_LIBS p3dconfig:c p3prc:c p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3dtoolconfig:m p3dtool:m

#begin metalib_target
  #define TARGET pandaexpress

  #define SOURCES pandaexpress.cxx
  #define WIN_SYS_LIBS \
     advapi32.lib ws2_32.lib $[WIN_SYS_LIBS]

#end metalib_target
