#define UNIX_SYS_LIBS m
#define USE_PACKAGES fftw

#define OTHER_LIBS \
  egg:c pandaegg:m \
  pipeline:c pnmimage:c putil:c event:c mathutil:c linmath:c panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub


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
