#define USE_PACKAGES fftw cg
#define OTHER_LIBS \
  p3egg2pg:c p3egg:c pandaegg:m \
  p3pipeline:c p3recorder:c p3parametrics:c p3collide:c p3chan:c p3char:c \
  p3dgraph:c p3downloader:c p3recorder:c \
  p3pnmimagetypes:c p3pnmimage:c p3pgraph:c p3display:c \
  p3pgraphnodes:c p3gobj:c p3putil:c \
  p3mathutil:c p3linmath:c p3event:c p3pstatclient:c \
  p3gsgbase:c p3grutil:c p3text:c p3cull:c \
  p3tform:c p3device:c p3movies:c \
  $[if $[HAVE_FREETYPE],p3pnmtext:c] \
  $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
  $[if $[HAVE_AUDIO],p3audio:c] \
  panda:m \
  p3pandabase:c p3express:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c \
  p3dtoolconfig:m p3dtool:m p3pystub

#begin bin_target
  #define TARGET bam-info
  #define LOCAL_LIBS \
    p3progbase

  #define SOURCES \
    bamInfo.cxx bamInfo.h

  #define INSTALL_HEADERS
#end bin_target

#begin bin_target
  #define TARGET egg2bam
  #define LOCAL_LIBS \
    p3eggbase p3progbase

  #define SOURCES \
    eggToBam.cxx eggToBam.h
#end bin_target


#begin bin_target
  #define TARGET bam2egg
  #define LOCAL_LIBS \
    p3converter p3eggbase p3progbase

  #define SOURCES \
    bamToEgg.cxx bamToEgg.h
#end bin_target

#begin bin_target
  #define TARGET pts2bam
  #define LOCAL_LIBS \
   p3progbase

  #define SOURCES \
    ptsToBam.cxx ptsToBam.h
#end bin_target
