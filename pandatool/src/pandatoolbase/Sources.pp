#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m express:c putil:c

#begin ss_lib_target
  #define TARGET pandatoolbase
  
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
