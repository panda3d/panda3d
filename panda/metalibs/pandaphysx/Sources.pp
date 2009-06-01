// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAPHYSX
#define BUILD_DIRECTORY $[HAVE_PHYSX]

#if $[eq $[LINK_IN_PHYSX],]
  // We don't have any components if we're linking the Physics library
  // directly into Panda.
  #define COMPONENT_LIBS \
      physx
#endif

#define LOCAL_LIBS linmath putil express
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolbase:c dtoolutil:c dtool:m prc:c

#begin metalib_target
  #define TARGET pandaphysx

  #define SOURCES pandaphysx.cxx pandaphysx.h
  #define INSTALL_HEADERS pandaphysx.h

#end metalib_target
