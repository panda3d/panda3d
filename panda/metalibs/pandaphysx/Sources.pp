// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILD_DIRECTORY $[HAVE_PHYSX]
#define BUILDING_DLL BUILDING_PANDAPHYSX

#define EXTRA_CDEFS NX32 $[if $[eq $[PLATFORM], Linux], LINUX CORELIB NX_DISABLE_FLUIDS]

#define COMPONENT_LIBS physx
#define LOCAL_LIBS linmath putil express
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin metalib_target
    #define TARGET pandaphysx
    #define SOURCES pandaphysx.cxx pandaphysx.h
    #define INSTALL_HEADERS pandaphysx.h
#end metalib_target
