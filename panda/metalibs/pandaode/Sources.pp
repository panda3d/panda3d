// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILD_DIRECTORY $[HAVE_ODE]
#define BUILDING_DLL BUILDING_PANDAODE
 
#define COMPONENT_LIBS \
    p3ode

#define LOCAL_LIBS p3pgraph
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
	           p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c


#begin metalib_target
#define TARGET pandaode

  #define SOURCES pandaode.cxx pandaode.h
  #define INSTALL_HEADERS pandaode.h

#end metalib_target

