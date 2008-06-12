#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#define USE_PACKAGES freetype

#define LOCAL_LIBS \
  palettizer eggbase progbase
#define OTHER_LIBS \
  egg:c pandaegg:m \
  display:c pnmimagetypes:c pnmimage:c \
  linmath:c putil:c pgraph:c pipeline:c cull:c \
  gsgbase:c gobj:c event:c mathutil:c pstatclient:c \
  lerp:c \
  $[if $[HAVE_FREETYPE],pnmtext:c] \
  $[if $[HAVE_NET],net:c] \
  panda:m \
  downloader:c pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET egg-mkfont
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx   
  
  #defer SOURCES \
    eggMakeFont.h \
    rangeDescription.h rangeDescription.I \
    rangeIterator.h rangeIterator.I

  #define INCLUDED_SOURCES \
    eggMakeFont.cxx \
    rangeDescription.cxx \
    rangeIterator.cxx

#end bin_target
