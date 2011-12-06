// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILD_DIRECTORY $[HAVE_BULLET]
#define BUILDING_DLL BUILDING_PANDABULLET

#define COMPONENT_LIBS p3bullet
#define LOCAL_LIBS p3linmath p3putil p3express
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin metalib_target
    #define TARGET pandabullet
    #define SOURCES pandabullet.cxx pandabullet.h
    #define INSTALL_HEADERS pandabullet.h
#end metalib_target
