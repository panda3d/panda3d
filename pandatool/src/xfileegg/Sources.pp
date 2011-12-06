#begin ss_lib_target
  #define TARGET p3xfileegg
  #define LOCAL_LIBS p3xfile p3eggbase p3progbase p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3mathutil:c p3linmath:c p3putil:c p3pipeline:c p3event:c \
    p3pnmimage:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     
    
  #define SOURCES \
     xFileAnimationSet.h xFileAnimationSet.I \
     xFileFace.h xFileMaker.h xFileMaterial.h \
     xFileMesh.h xFileNormal.h \
     xFileToEggConverter.h xFileVertex.h 

  #define INCLUDED_SOURCES \
     xFileAnimationSet.cxx \
     xFileFace.cxx xFileMaker.cxx xFileMaterial.cxx \
     xFileMesh.cxx xFileNormal.cxx \
     xFileToEggConverter.cxx xFileVertex.cxx 

#end ss_lib_target
