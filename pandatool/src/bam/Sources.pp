#define USE_PACKAGES fftw
#define OTHER_LIBS \
  egg2pg:c egg:c pandaegg:m \
  pipeline:c recorder:c parametrics:c collide:c chan:c char:c \
  dgraph:c downloader:c recorder:c \
  pnmimagetypes:c pnmimage:c pgraph:c gobj:c putil:c \
  lerp:c mathutil:c linmath:c event:c pstatclient:c \
  gsgbase:c grutil:c display:c text:c cull:c pnmtext:c \
  tform:c device:c \
  $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
  panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c \
  dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET bam-info
  #define LOCAL_LIBS \
    progbase

  #define SOURCES \
    bamInfo.cxx bamInfo.h

  #define INSTALL_HEADERS
#end bin_target

#begin bin_target
  #define TARGET egg2bam
  #define LOCAL_LIBS \
    eggbase progbase

  #define SOURCES \
    eggToBam.cxx eggToBam.h
#end bin_target


#begin bin_target
  #define TARGET bam2egg
  #define LOCAL_LIBS \
    converter eggbase progbase

  #define SOURCES \
    bamToEgg.cxx bamToEgg.h
#end bin_target
