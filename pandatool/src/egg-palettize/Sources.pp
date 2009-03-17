#define USE_PACKAGES cg  // from gobj.

#define OTHER_LIBS \
  egg:c pandaegg:m \
  pgraph:c pgraphnodes:c gobj:c lerp:c linmath:c putil:c \
  pnmimage:c pnmimagetypes:c display:c pipeline:c \
  event:c mathutil:c cull:c gsgbase:c pstatclient:c \
  $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
  panda:m \
  pandabase:c express:c downloader:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c \
  dtoolconfig:m dtool:m 

#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    palettizer eggbase progbase

  #define OTHER_LIBS $[OTHER_LIBS] pystub
  
  #define SOURCES \
     eggPalettize.h eggPalettize.cxx

#end bin_target

#begin lib_target
  #define TARGET txafile
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    palettizer

  #define SOURCES \
    txaFileFilter.h txaFileFilter.I txaFileFilter.cxx

#end lib_target

