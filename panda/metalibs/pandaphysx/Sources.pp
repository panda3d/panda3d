// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILD_DIRECTORY $[HAVE_PHYSX]
#define BUILDING_DLL BUILDING_PANDAPHYSX

#define COMPONENT_LIBS p3physx
#define LOCAL_LIBS p3linmath p3putil p3express
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin metalib_target
    #define TARGET pandaphysx
    #define SOURCES pandaphysx.cxx pandaphysx.h
    #define INSTALL_HEADERS pandaphysx.h
#end metalib_target
