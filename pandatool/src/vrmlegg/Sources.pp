#begin ss_lib_target
  #define TARGET p3vrmlegg
  #define LOCAL_LIBS p3converter p3vrml p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3mathutil:c p3linmath:c p3event:c p3putil:c p3express:c \
    p3pipeline:c \
    panda:m \
    p3pandabase:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m

  #define SOURCES \
    indexedFaceSet.cxx indexedFaceSet.h \
    vrmlAppearance.cxx vrmlAppearance.h \
    vrmlToEggConverter.cxx vrmlToEggConverter.h

  #define INSTALL_HEADERS \
    indexedFaceSet.h \
    vrmlAppearance.h \
    vrmlToEggConverter.h

#end ss_lib_target
