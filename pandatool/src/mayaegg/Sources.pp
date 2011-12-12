#define BUILD_DIRECTORY $[HAVE_MAYA]

#begin ss_lib_target
  #define USE_PACKAGES maya
  #define TARGET p3mayaegg
  #define LOCAL_LIBS \
    mayabase p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub \
    p3pipeline:c p3interrogatedb:c p3gobj:c

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_mayaegg.cxx config_mayaegg.h \
    mayaEggGroupUserData.cxx mayaEggGroupUserData.I mayaEggGroupUserData.h \
    mayaEggLoader.cxx mayaEggLoader.h \
    mayaBlendDesc.cxx mayaBlendDesc.h \
    mayaNodeDesc.cxx mayaNodeDesc.h \
    mayaNodeTree.cxx mayaNodeTree.h \
    mayaToEggConverter.cxx mayaToEggConverter.h

#end ss_lib_target

