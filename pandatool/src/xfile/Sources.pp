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
    xFileFace.cxx xFileFace.h \
    xFileMaker.cxx xFileMaker.h \
    xFileMaterial.cxx xFileMaterial.h \
    xFileMesh.cxx xFileMesh.h \
    xFileNormal.cxx xFileNormal.h \
    xFileTemplates.cxx xFileTemplates.h \
    xFileToEggConverter.cxx xFileToEggConverter.h \
    xFileVertex.cxx xFileVertex.h

  #define SOURCES \
    $[SOURCES] \
    eggToX.cxx eggToX.h

#end bin_target

#begin bin_target
  #define TARGET x2egg
  #define LOCAL_LIBS converter eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib

  #define SOURCES \
    xFileFace.cxx xFileFace.h \
    xFileMaker.cxx xFileMaker.h \
    xFileMaterial.cxx xFileMaterial.h \
    xFileMesh.cxx xFileMesh.h \
    xFileNormal.cxx xFileNormal.h \
    xFileTemplates.cxx xFileTemplates.h \
    xFileToEggConverter.cxx xFileToEggConverter.h \
    xFileVertex.cxx xFileVertex.h

  #define SOURCES \
    $[SOURCES] \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target
