#define DIRECTORY_IF_MAYA yes

#begin ss_lib_target
  #define USE_MAYA yes
  #define TARGET mayaegg
  #define LOCAL_LIBS \
    converter pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_mayaegg.cxx config_mayaegg.h \
    mayaApi.cxx mayaApi.h \
    mayaParameters.cxx mayaParameters.h \
    mayaShader.cxx mayaShader.h \
    mayaShaders.cxx mayaShaders.h \
    mayaToEggConverter.cxx mayaToEggConverter.h \
    maya_funcs.I maya_funcs.cxx maya_funcs.h \
    post_maya_include.h pre_maya_include.h

#end ss_lib_target

