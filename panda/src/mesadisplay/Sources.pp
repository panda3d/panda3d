#define BUILD_DIRECTORY $[HAVE_MESA]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl mesa

#begin lib_target
  #define TARGET mesadisplay
  #define LOCAL_LIBS \
    glgsg

  #define SOURCES \
    config_mesadisplay.cxx config_mesadisplay.h \
    mesaGraphicsBuffer.I mesaGraphicsBuffer.cxx mesaGraphicsBuffer.h \
    mesaGraphicsPipe.I mesaGraphicsPipe.cxx mesaGraphicsPipe.h \
    mesaGraphicsStateGuardian.h mesaGraphicsStateGuardian.cxx \
    mesaGraphicsStateGuardian.I

  #define IGATESCAN mesaGraphicsPipe.h

#end lib_target

