#define BUILD_DIRECTORY $[HAVE_SOFTIMAGE]

#begin lib_target
  #define USE_PACKAGES softimage
  #define TARGET softegg
  #define LOCAL_LIBS \
    converter pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

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

