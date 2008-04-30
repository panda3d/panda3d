#define BUILD_DIRECTORY $[HAVE_TINYSDGL]
//#define BUILDING_DLL BUILDING_PANDATINYSDGL

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES tinysdgl

#begin lib_target
  #define TARGET tinydisplay
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define SOURCES \
    config_tinydisplay.cxx config_tinydisplay.h \
    tinyGeomMunger.I tinyGeomMunger.cxx tinyGeomMunger.h \
    tinyGraphicsPipe.I tinyGraphicsPipe.cxx tinyGraphicsPipe.h \
    tinyGraphicsWindow.h tinyGraphicsWindow.I tinyGraphicsWindow.cxx \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyGraphicsStateGuardian.cxx \
    tinyImmediateModeSender.h tinyImmediateModeSender.I \
    tinyImmediateModeSender.cxx \
    tinyTextureContext.I tinyTextureContext.cxx tinyTextureContext.h 

  #define INSTALL_HEADERS \
    tinyGeomMunger.I tinyGeomMunger.h \
    tinyGraphicsPipe.I tinyGraphicsPipe.h \
    tinyGraphicsWindow.I tinyGraphicsWindow.h \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyImmediateModeSender.h tinyImmediateModeSender.I \
    tinyTextureContext.I tinyTextureContext.h 

#end lib_target

