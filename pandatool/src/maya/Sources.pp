#define DIRECTORY_IF_MAYA yes

#begin sed_bin_target
  #define TARGET maya2egg

  #define SOURCE maya2egg_script
  #define COMMAND 's:xxx:$[MAYA_LOCATION]:g'

#end sed_bin_target

#begin bin_target
  #define USE_MAYA yes
  #define TARGET maya2egg_bin
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c linmath:c putil:c express:c panda:m \
    dtoolutil:c dconfig:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    global_parameters.cxx global_parameters.h mayaFile.cxx mayaFile.h \
    mayaShader.cxx mayaShader.h mayaShaders.cxx mayaShaders.h \
    mayaToEgg.cxx mayaToEgg.h maya_funcs.I maya_funcs.cxx maya_funcs.h

#end bin_target

