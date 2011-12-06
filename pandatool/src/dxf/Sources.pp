#begin ss_lib_target
  #define TARGET p3dxf
  #define LOCAL_LIBS p3pandatoolbase

  #define OTHER_LIBS \
    p3pipeline:c p3event:c p3putil:c p3mathutil:c p3linmath:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3dtoolutil:c p3dconfig:c p3prc:c p3interrogatedb:c p3dtoolbase:c p3dtool:m
  
  #define SOURCES \
    dxfFile.cxx dxfFile.h dxfLayer.h dxfLayer.cxx \
    dxfLayerMap.cxx dxfLayerMap.h \
    dxfVertex.cxx dxfVertex.h

  #define INSTALL_HEADERS \
    dxfFile.h dxfLayer.h dxfLayerMap.h dxfVertex.h

#end ss_lib_target
