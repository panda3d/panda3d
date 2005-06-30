#define BUILD_DIRECTORY $[HAVE_MESA]
#define BUILDING_DLL BUILDING_PANDAMESA

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES mesa cg cggl

#begin lib_target
  #define TARGET mesadisplay
  #define LOCAL_LIBS \
    glstuff gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define SOURCES \
    config_mesadisplay.cxx config_mesadisplay.h \
    mesagsg.h mesagsg.cxx \
    osMesaGraphicsBuffer.I osMesaGraphicsBuffer.cxx osMesaGraphicsBuffer.h \
    osMesaGraphicsPipe.I osMesaGraphicsPipe.cxx osMesaGraphicsPipe.h \
    osMesaGraphicsStateGuardian.h osMesaGraphicsStateGuardian.cxx \
    osMesaGraphicsStateGuardian.I

#end lib_target

