#define UNIX_SYS_LIBS m
#define USE_PACKAGES fftw

#begin bin_target
  #define TARGET dxf-points
  #define LOCAL_LIBS \
    progbase dxf
  #define OTHER_LIBS \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    dxfPoints.cxx dxfPoints.h

#end bin_target

#begin bin_target
  #define TARGET dxf2egg
  #define LOCAL_LIBS dxf dxfegg eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c pnmimagetypes:c pnmimage:c putil:c mathutil:c event:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    dxfToEgg.cxx dxfToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2dxf
  #define LOCAL_LIBS dxf eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    putil:c event:c linmath:c pnmimagetypes:c pnmimage:c mathutil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    eggToDXF.cxx eggToDXF.h \
    eggToDXFLayer.cxx eggToDXFLayer.h

#end bin_target
