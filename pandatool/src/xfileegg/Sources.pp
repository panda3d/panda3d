
// The .x converter takes advantage of the DirectX API's; therefore,
// it can only be built if we have DX available (and therefore it only
// builds on Windows--sorry).
#define BUILD_DIRECTORY $[HAVE_DX]
#define USE_PACKAGES dx

#begin ss_lib_target
  #define TARGET xfileegg
  #define LOCAL_LIBS xfile eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib dxguid.lib d3d8.lib d3dx8.lib dxerr8.lib
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     
    
  #define SOURCES \
     xFileAnimationSet.h \
     xFileFace.h xFileMaker.h xFileMaterial.h \
     xFileMesh.h xFileNormal.h xFileTemplates.h \
     xFileToEggConverter.h xFileVertex.h 

  #define INCLUDED_SOURCES \
     xFileAnimationSet.cxx \
     xFileFace.cxx xFileMaker.cxx xFileMaterial.cxx \
     xFileMesh.cxx xFileNormal.cxx xFileTemplates.cxx \
     xFileToEggConverter.cxx xFileVertex.cxx 

#end ss_lib_target
