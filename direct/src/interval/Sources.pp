#begin lib_target
  #define TARGET p3interval
  #define LOCAL_LIBS \
    p3directbase
  #define OTHER_LIBS \
    p3downloader:c p3linmath:c \
    p3chan:c p3event:c p3gobj:c p3pnmimage:c p3mathutil:c \
    p3pgraph:c p3putil:c panda:m p3express:c pandaexpress:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3pandabase:c p3prc:c p3gsgbase:c p3pstatclient:c \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3pipeline:c

  #define SOURCES \
    config_interval.cxx config_interval.h \
    cInterval.cxx cInterval.I cInterval.h \
    cIntervalManager.cxx cIntervalManager.I cIntervalManager.h \
    cConstraintInterval.cxx cConstraintInterval.I cConstraintInterval.h \
    cConstrainTransformInterval.cxx cConstrainTransformInterval.I cConstrainTransformInterval.h \
    cConstrainPosInterval.cxx cConstrainPosInterval.I cConstrainPosInterval.h \
    cConstrainHprInterval.cxx cConstrainHprInterval.I cConstrainHprInterval.h \
    cConstrainPosHprInterval.cxx cConstrainPosHprInterval.I cConstrainPosHprInterval.h \
    cLerpInterval.cxx cLerpInterval.I cLerpInterval.h \
    cLerpNodePathInterval.cxx cLerpNodePathInterval.I cLerpNodePathInterval.h \
    cLerpAnimEffectInterval.cxx cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h \
    cMetaInterval.cxx cMetaInterval.I cMetaInterval.h \
    hideInterval.cxx hideInterval.I hideInterval.h \
    lerpblend.cxx lerpblend.h \
    showInterval.cxx showInterval.I showInterval.h \
    waitInterval.cxx waitInterval.I waitInterval.h \
    lerp_helpers.h

  #define INSTALL_HEADERS \
    config_interval.h \
    cInterval.I cInterval.h \
    cIntervalManager.I cIntervalManager.h \
    cConstraintInterval.I cConstraintInterval.h \
    cConstrainTransformInterval.I cConstrainTransformInterval.h \
    cConstrainPosInterval.I cConstrainPosInterval.h \
    cConstrainHprInterval.I cConstrainHprInterval.h \
    cConstrainPosHprInterval.I cConstrainPosHprInterval.h \
    cLerpInterval.I cLerpInterval.h \
    cLerpNodePathInterval.I cLerpNodePathInterval.h \
    cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h \
    cMetaInterval.I cMetaInterval.h \
    hideInterval.I hideInterval.h \
    lerpblend.h \
    showInterval.I showInterval.h \
    waitInterval.I waitInterval.h \
    lerp_helpers.h

  #define IGATESCAN all
#end lib_target
