#define BUILD_DIRECTORY $[HAVE_RIB]

// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDARIB

#define COMPONENT_LIBS \
    ribgsg ribdisplay
#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS dtoolconfig dtool

#begin metalib_target
  #define TARGET pandarib

  #define SOURCES pandarib.cxx
#end metalib_target
