// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDA
#define USE_PACKAGES net

#define COMPONENT_LIBS \
    recorder pgraph pgraphnodes pipeline \
    pvrpn grutil chan pstatclient \
    char collide cull device \
    dgraph display event gobj gsgbase \
    linmath mathutil movies net nativenet\
    parametrics \
    pnmimagetypes pnmimage \
    pnmtext text tform lerp putil \
    audio pgui pandabase helix

#define LOCAL_LIBS \
  downloader express pandabase
#define OTHER_LIBS \
  pandaexpress:m \
  interrogatedb:c dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c dtool:m prc:c

#if $[LINK_IN_PHYSX]
  #define BUILDING_DLL $[BUILDING_DLL] BUILDING_PANDAPHYSX
  #define COMPONENT_LIBS $[COMPONENT_LIBS] physx
#endif

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx panda.h
  #define INSTALL_HEADERS panda.h
#end metalib_target














