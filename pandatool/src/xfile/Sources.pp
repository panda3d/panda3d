#define DIRECTORY_IF_DX yes
#define USE_DX yes

#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib

  #define SOURCES \
    xFileMaker.cxx xFileMaker.h

  #define SOURCES \
    $[SOURCES] \
    eggToX.cxx eggToX.h

#end bin_target
