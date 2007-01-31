#begin lib_target
  #define TARGET motiontrail
  #define LOCAL_LIBS \
    directbase

  #define OTHER_LIBS \
    linmath:c \
    mathutil:c \
    gobj:c \
    panda:m \
    express:c \
    pandaexpress:m \
    interrogatedb:c \
    dconfig:c \
    dtoolconfig:m \
    dtoolutil:c \
    dtoolbase:c \
    dtool:m \
    pandabase:c \
    prc:c \
    gsgbase:c \
    parametrics:c

  
  #define SOURCES \
    config_motiontrail.cxx config_motiontrail.h \
    cMotionTrail.cxx cMotionTrail.h 

  #define INSTALL_HEADERS \
    config_motiontrail.h \
    cMotionTrail.h 

  #define IGATESCAN all
#end lib_target
