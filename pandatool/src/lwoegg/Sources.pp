#begin ss_lib_target
  #define TARGET lwoegg
  #define LOCAL_LIBS converter lwo pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c express:c panda:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS m

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx   
  
  #define SOURCES \
     cLwoClip.I cLwoClip.h cLwoLayer.I cLwoLayer.h cLwoPoints.I  \
     cLwoPoints.h cLwoPolygons.I cLwoPolygons.h cLwoSurface.I  \
     cLwoSurface.h cLwoSurfaceBlock.I cLwoSurfaceBlock.h  \
     cLwoSurfaceBlockTMap.I cLwoSurfaceBlockTMap.h  \
     lwoToEggConverter.I lwoToEggConverter.h 
    
  #define INCLUDED_SOURCES \
     cLwoClip.cxx cLwoLayer.cxx cLwoPoints.cxx cLwoPolygons.cxx  \
     cLwoSurface.cxx cLwoSurfaceBlock.cxx  \
     cLwoSurfaceBlockTMap.cxx lwoToEggConverter.cxx  \
     lwoToEggConverter.h 

  #define INSTALL_HEADERS \
    lwoToEggConverter.I lwoToEggConverter.h

#end ss_lib_target
