#begin ss_lib_target
  #define TARGET dxf
  #define LOCAL_LIBS pandatoolbase

  #define OTHER_LIBS \
    mathutil:c linmath:c panda:m \
    dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS m
  
  #define SOURCES \
    dxfFile.cxx dxfFile.h dxfLayer.h dxfLayer.cxx \
    dxfLayerMap.cxx dxfLayerMap.h \
    dxfVertex.cxx dxfVertex.h

  #define INSTALL_HEADERS \
    dxfFile.h dxfLayer.h dxfLayerMap.h dxfVertex.h

#end ss_lib_target
