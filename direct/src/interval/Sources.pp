#begin lib_target
  #define TARGET interval
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    pgraph:c putil:c panda:m express:c pandaexpress:m dtoolconfig dtool

  #define SOURCES \
    config_interval.cxx config_interval.h \
    cInterval.cxx cInterval.I cInterval.h \
    cLerpInterval.cxx cLerpInterval.I cLerpInterval.h \
    cLerpNodePathInterval.cxx cLerpNodePathInterval.I cLerpNodePathInterval.h \
    cLerpAnimEffectInterval.cxx cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h \
    cMetaInterval.cxx cMetaInterval.I cMetaInterval.h \
    showInterval.cxx showInterval.I showInterval.h \
    hideInterval.cxx hideInterval.I hideInterval.h \
    lerp_helpers.h

  #define IGATESCAN all
#end lib_target
