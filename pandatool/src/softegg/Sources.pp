#define BUILD_DIRECTORY $[HAVE_SOFTIMAGE]

#begin lib_target
  #define USE_PACKAGES softimage
  #define TARGET p3softegg
  #define LOCAL_LIBS \
    p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m

  #define BUILDING_DLL BUILDING_MISC
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_softegg.cxx config_softegg.h \
    softEggGroupUserData.cxx softEggGroupUserData.I softEggGroupUserData.h \
    softToEggConverter.cxx softToEggConverter.h \
    soft2Egg.c \
    softNodeTree.cxx softNodeTree.h \
    softNodeDesc.cxx softNodeDesc.h

#end lib_target

