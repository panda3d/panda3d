
// The .x converter takes advantage of the DirectX API's; therefore,
// it can only be built if we have DX available (and therefore it only
// builds on Windows--sorry).
#define BUILD_DIRECTORY $[HAVE_DX]
#define USE_PACKAGES dx

#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS xfile eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib dxguid.lib d3d8.lib d3dx8.lib dxerr8.lib

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
    d3dxof.lib dxguid.lib d3d8.lib d3dx8.lib dxerr8.lib

  #define SOURCES \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target
