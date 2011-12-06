#define BUILD_DIRECTORY $[HAVE_MESA]
#define BUILDING_DLL BUILDING_PANDAMESA

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m
#define USE_PACKAGES mesa cg cggl

#begin lib_target
  #define TARGET p3mesadisplay
  #define LOCAL_LIBS \
    p3glstuff p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage

  #define SOURCES \
    config_mesadisplay.cxx config_mesadisplay.h \
    mesagsg.h mesagsg.cxx \
    osMesaGraphicsBuffer.I osMesaGraphicsBuffer.cxx osMesaGraphicsBuffer.h \
    osMesaGraphicsPipe.I osMesaGraphicsPipe.cxx osMesaGraphicsPipe.h \
    osMesaGraphicsStateGuardian.h osMesaGraphicsStateGuardian.cxx \
    osMesaGraphicsStateGuardian.I

#end lib_target

