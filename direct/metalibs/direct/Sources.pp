// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_DIRECT

#define COMPONENT_LIBS \
  directbase dcparser showbase deadrec directd interval distributed

#define OTHER_LIBS \
  panda:m \
  pandaexpress:m \
  interrogatedb:c dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c dtool:m \
  express:c pstatclient:c prc:c pandabase:c linmath:c \
  putil:c display:c event:c pgraph:c gsgbase:c gobj:c mathutil:c \
  lerp:c downloader:c pnmimage:c chan:c \
  pipeline:c cull:c $[if $[HAVE_NET],net:c]

#begin metalib_target
  #define TARGET direct

  #define SOURCES direct.cxx
#end metalib_target

