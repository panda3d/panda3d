#define BUILD_DIRECTORY $[HAVE_CHROMIUM]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES chromium

#begin lib_target
  #define TARGET crgsg
  #define LOCAL_LIBS \
    pandabase gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
     crGraphicsStateGuardian.cxx \
     config_crgsg.h crGraphicsStateGuardian.I \
     crGraphicsStateGuardian.h crSavedFrameBuffer.I \
     crSavedFrameBuffer.h crTextureContext.I crext.h \
     crGeomNodeContext.I crGeomNodeContext.h  crTextureContext.h

  #define INCLUDED_SOURCES \
     config_crgsg.cxx crSavedFrameBuffer.cxx \
     crGeomNodeContext.cxx crTextureContext.cxx 

  #define INSTALL_HEADERS \
    config_crgsg.h crGraphicsStateGuardian.I crGraphicsStateGuardian.h

#end lib_target

