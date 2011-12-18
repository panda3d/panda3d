// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_DIRECT
#define USE_PACKAGES native_net

#define COMPONENT_LIBS \
  p3directbase p3dcparser p3showbase p3deadrec p3directd p3interval p3distributed p3motiontrail p3http

#define OTHER_LIBS \
  panda:m \
  pandaexpress:m \
  p3parametrics:c \
  p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
  p3dtoolutil:c p3dtoolbase:c p3dtool:m \
  p3express:c p3pstatclient:c p3prc:c p3pandabase:c p3linmath:c \
  p3putil:c p3display:c p3event:c p3pgraph:c p3pgraphnodes:c \
  p3gsgbase:c p3gobj:c p3mathutil:c \
  p3downloader:c p3pnmimage:c p3chan:c \
  p3pipeline:c p3cull:c \
  $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c]

#begin metalib_target
  #define TARGET p3direct

  #define SOURCES direct.cxx
#end metalib_target

