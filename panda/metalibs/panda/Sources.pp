// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDA
#define USE_NET yes

#define COMPONENT_LIBS \
    pstatclient grutil chan chancfg \
    char chat collide cull device \
    dgraph display gobj graph gsgbase \
    gsgmisc light linmath mathutil net pnm \
    pnmimagetypes pnmimage sgattrib sgmanip sgraph sgraphutil \
    switchnode text tform tiff lerp loader putil effects \
    audio pandabase 

#define LOCAL_LIBS downloader event ipc express pandabase
#define OTHER_LIBS dtool

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx
#end metalib_target
