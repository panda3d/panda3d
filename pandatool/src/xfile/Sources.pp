#define DIRECTORY_IF_DX yes
#define USE_DX yes

#begin ss_lib_target
  #define TARGET xfile
  #define LOCAL_LIBS eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define WIN_SYS_LIBS \
    d3dxof.lib dxguid.lib

  #define SOURCES \
    config_xfile.cxx config_xfile.h \
    xFileFace.cxx xFileFace.h \
    xFileMaker.cxx xFileMaker.h \
    xFileMaterial.cxx xFileMaterial.h \
    xFileMesh.cxx xFileMesh.h \
    xFileNormal.cxx xFileNormal.h \
    xFileTemplates.cxx xFileTemplates.h \
    xFileToEggConverter.cxx xFileToEggConverter.h \
    xFileVertex.cxx xFileVertex.h

#end ss_lib_target
