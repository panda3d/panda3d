// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDA

#define LOCAL_LIBS \
    pstatclient grutil chan chancfg \
    char chat collide cull device \
    dgraph display gobj graph gsgbase \
    gsgmisc light linmath mathutil net pnm \
    pnmimagetypes pnmimage sgattrib sgmanip sgraph sgraphutil \
    switchnode text tform lerp loader putil effects \
    audio pandabase

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx
#end metalib_target
