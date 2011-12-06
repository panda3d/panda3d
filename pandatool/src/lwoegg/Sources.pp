#begin ss_lib_target
  #define TARGET p3lwoegg
  #define LOCAL_LIBS p3converter p3lwo p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3mathutil:c p3linmath:c p3putil:c p3pipeline:c p3event:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m
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
