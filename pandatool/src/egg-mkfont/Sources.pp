#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#define USE_PACKAGES freetype

#define LOCAL_LIBS \
  p3palettizer p3eggbase p3progbase

#define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3pnmimagetypes:c p3pnmtext:c \
    p3mathutil:c p3linmath:c p3putil:c p3express:c \
    pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c] \
    p3pystub

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
