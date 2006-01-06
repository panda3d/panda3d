#begin lib_target
  #define TARGET interval
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    downloader:c linmath:c lerp:c \
    chan:c event:c gobj:c pnmimage:c mathutil:c \
    pgraph:c putil:c panda:m express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define SOURCES \
    config_interval.cxx config_interval.h \
    cInterval.cxx cInterval.I cInterval.h \
    cIntervalManager.cxx cIntervalManager.I cIntervalManager.h \
    cLerpInterval.cxx cLerpInterval.I cLerpInterval.h \
    cLerpNodePathInterval.cxx cLerpNodePathInterval.I cLerpNodePathInterval.h \
    cLerpAnimEffectInterval.cxx cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h \
    cMetaInterval.cxx cMetaInterval.I cMetaInterval.h \
    hideInterval.cxx hideInterval.I hideInterval.h \
    showInterval.cxx showInterval.I showInterval.h \
    waitInterval.cxx waitInterval.I waitInterval.h \
    lerp_helpers.h

  #define INSTALL_HEADERS \
    config_interval.h \
    cInterval.I cInterval.h \
    cIntervalManager.I cIntervalManager.h \
    cLerpInterval.I cLerpInterval.h \
    cLerpNodePathInterval.I cLerpNodePathInterval.h \
    cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h \
    cMetaInterval.I cMetaInterval.h \
    hideInterval.I hideInterval.h \
    showInterval.I showInterval.h \
    waitInterval.I waitInterval.h \
    lerp_helpers.h

  #define IGATESCAN all
#end lib_target
