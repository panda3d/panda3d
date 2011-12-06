#define BUILD_DIRECTORY $[HAVE_FCOLLADA]

#begin ss_lib_target
  #define USE_PACKAGES fcollada
  #define TARGET p3daeegg
  #define LOCAL_LIBS p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3pipeline:c p3mathutil:c p3linmath:c p3putil:c p3event:c \
    panda:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m

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

#end ss_lib_target
