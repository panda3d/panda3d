// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAGLES2
#define BUILD_DIRECTORY $[HAVE_GLES2]

#define COMPONENT_LIBS \
    p3gles2gsg egl2display

#define LOCAL_LIBS p3gsgbase p3display p3express
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin metalib_target
  #define TARGET pandagles2
  #define SOURCES pandagles2.cxx pandagles2.h
  #define INSTALL_HEADERS pandagles2.h
#end metalib_target
