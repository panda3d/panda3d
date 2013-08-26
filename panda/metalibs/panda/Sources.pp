// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDA
#define USE_PACKAGES net

#define COMPONENT_LIBS \
    p3recorder p3pgraph p3pgraphnodes p3pipeline \
    p3grutil p3chan p3pstatclient \
    p3char p3collide p3cull p3device \
    p3dgraph p3display p3event p3gobj p3gsgbase \
    p3linmath p3mathutil p3movies p3net p3nativenet \
    p3parametrics \
    p3pnmimagetypes p3pnmimage \
    p3pnmtext p3text p3tform p3putil \
    p3audio p3pgui p3pandabase p3dxml

#define LOCAL_LIBS \
  p3downloader p3express p3pandabase
#define OTHER_LIBS \
  pandaexpress:m \
  p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
  p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#if $[LINK_IN_PHYSX]
  #define BUILDING_DLL $[BUILDING_DLL] BUILDING_PANDAPHYSX
  #define COMPONENT_LIBS $[COMPONENT_LIBS] p3physx
#endif

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx panda.h
  #define INSTALL_HEADERS panda.h
#end metalib_target

