// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_DTOOLCONFIG

#define COMPONENT_LIBS interrogatedb dconfig
#define LOCAL_LIBS dtoolutil dtoolbase
#define USE_PACKAGES python

#begin metalib_target
  #define TARGET dtoolconfig

  #define SOURCES dtoolconfig.cxx \
     $[if $[HAVE_PYTHON], pydtool.cxx]
#end metalib_target
