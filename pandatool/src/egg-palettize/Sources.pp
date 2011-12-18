#define USE_PACKAGES cg  // from gobj.

#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3pgraph:c p3pgraphnodes:c p3gobj:c p3linmath:c p3putil:c \
  p3pnmimage:c p3pnmimagetypes:c p3display:c p3pipeline:c \
  p3event:c p3mathutil:c p3cull:c p3gsgbase:c p3pstatclient:c \
  $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
  panda:m \
  p3pandabase:c p3express:c p3downloader:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c \
  p3dtoolconfig:m p3dtool:m 

#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    p3palettizer p3eggbase p3progbase

  #define OTHER_LIBS $[OTHER_LIBS] p3pystub
  
  #define SOURCES \
     eggPalettize.h eggPalettize.cxx

#end bin_target

#begin lib_target
  #define TARGET p3txafile
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    p3palettizer

  #define SOURCES \
    txaFileFilter.h txaFileFilter.I txaFileFilter.cxx

#end lib_target

