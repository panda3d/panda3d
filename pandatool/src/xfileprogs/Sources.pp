
// This package is temporarily disabled.
#define BUILD_DIRECTORY
//#define BUILD_DIRECTORY $[HAVE_DX]
#define USE_DX yes

#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS xfile eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib dxguid.lib

  #define SOURCES \
    eggToX.cxx eggToX.h

#end bin_target

#begin bin_target
  #define TARGET x2egg
  #define LOCAL_LIBS xfile converter eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib

  #define SOURCES \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target
