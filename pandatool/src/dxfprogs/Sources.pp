#define UNIX_SYS_LIBS m
#define USE_PACKAGES fftw

#define OTHER_LIBS \
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

#begin bin_target
  #define TARGET dxf-points
  #define LOCAL_LIBS \
    progbase dxf

  #define SOURCES \
    dxfPoints.cxx dxfPoints.h

#end bin_target

#begin bin_target
  #define TARGET dxf2egg
  #define LOCAL_LIBS dxf dxfegg eggbase progbase

  #define SOURCES \
    dxfToEgg.cxx dxfToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2dxf
  #define LOCAL_LIBS dxf eggbase progbase

  #define SOURCES \
    eggToDXF.cxx eggToDXF.h \
    eggToDXFLayer.cxx eggToDXFLayer.h

#end bin_target
