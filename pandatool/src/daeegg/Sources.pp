#define BUILD_DIRECTORY $[HAVE_FCOLLADA]

#begin lib_target
  #define USE_PACKAGES fcollada
  #define TARGET daeegg
  #define LOCAL_LIBS converter pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pandabase:c express:c pandaexpress:m \
    pipeline:c mathutil:c linmath:c putil:c event:c \
    panda:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define SOURCES \
    config_daeegg.cxx config_daeegg.h \
    daeToEggConverter.cxx daeToEggConverter.h \
    daeCharacter.cxx daeCharacter.h \
    daeMaterials.cxx daeMaterials.h \
    pre_fcollada_include.h fcollada_utils.h

  #define INSTALL_HEADERS \
    config_daeegg.h daeToEggConverter.h \
    daeCharacter.h daeMaterials.h \
    pre_fcollada_include.h fcollada_utils.h

#end lib_target
