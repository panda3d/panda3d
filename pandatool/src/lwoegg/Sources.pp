#begin ss_lib_target
  #define TARGET lwoegg
  #define LOCAL_LIBS converter lwo pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m dtoolconfig dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    cLwoClip.I cLwoClip.cxx cLwoClip.h \
    cLwoLayer.I cLwoLayer.cxx cLwoLayer.h \
    cLwoPoints.I cLwoPoints.cxx cLwoPoints.h \
    cLwoPolygons.I cLwoPolygons.cxx cLwoPolygons.h \
    cLwoSurface.I cLwoSurface.cxx cLwoSurface.h \
    cLwoSurfaceBlock.I cLwoSurfaceBlock.cxx cLwoSurfaceBlock.h \
    cLwoSurfaceBlockTMap.I cLwoSurfaceBlockTMap.cxx cLwoSurfaceBlockTMap.h \
    lwoToEggConverter.I lwoToEggConverter.cxx lwoToEggConverter.h

  #define INSTALL_HEADERS \
    lwoToEggConverter.I lwoToEggConverter.h

#end ss_lib_target
