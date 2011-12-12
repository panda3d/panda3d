#define BUILD_DIRECTORY $[HAVE_MAYA]

#begin ss_lib_target
  #define USE_PACKAGES maya
  #define TARGET mayabase
  #define LOCAL_LIBS \
    p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m \
    p3pipeline:c p3interrogatedb:c

  #define SOURCES \
    config_maya.cxx config_maya.h \
    mayaApi.cxx mayaApi.h \
    mayaShader.cxx mayaShader.h \
    mayaShaderColorDef.cxx mayaShaderColorDef.h \
    mayaShaders.cxx mayaShaders.h \
    maya_funcs.I maya_funcs.cxx maya_funcs.h \
    post_maya_include.h pre_maya_include.h

#end ss_lib_target

