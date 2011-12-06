#define OTHER_LIBS \
   p3pipeline:c panda:m \
   p3express:c p3putil:c p3pandabase:c pandaexpress:m \
   p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
   p3dtoolutil:c p3dtoolbase:c p3dtool:m

#begin ss_lib_target
  #define TARGET p3pandatoolbase
  
  #define SOURCES \
    animationConvert.cxx animationConvert.h \
    config_pandatoolbase.cxx config_pandatoolbase.h \
    distanceUnit.cxx distanceUnit.h \
    pandatoolbase.cxx pandatoolbase.h pandatoolsymbols.h \
    pathReplace.cxx pathReplace.I pathReplace.h \
    pathStore.cxx pathStore.h

  #define INSTALL_HEADERS \
    animationConvert.h \
    config_pandatoolbase.h \
    distanceUnit.h \
    pandatoolbase.h pandatoolsymbols.h \
    pathReplace.I pathReplace.h \
    pathStore.h

#end ss_lib_target
