#begin lib_target
  #define TARGET p3motiontrail
  #define LOCAL_LIBS \
    p3directbase

  #define OTHER_LIBS \
    p3linmath:c \
    p3mathutil:c \
    p3gobj:c \
    p3putil:c \
    p3pipeline:c \
    p3event:c \
    p3pstatclient:c \
    p3pnmimage:c \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3pgraph:c \
    panda:m \
    p3express:c \
    p3downloader:c \
    pandaexpress:m \
    p3interrogatedb:c \
    p3dconfig:c \
    p3dtoolconfig:m \
    p3dtoolutil:c \
    p3dtoolbase:c \
    p3dtool:m \
    p3pandabase:c \
    p3prc:c \
    p3gsgbase:c \
    p3parametrics:c

  
  #define SOURCES \
    config_motiontrail.cxx config_motiontrail.h \
    cMotionTrail.cxx cMotionTrail.h 

  #define INSTALL_HEADERS \
    config_motiontrail.h \
    cMotionTrail.h 

  #define IGATESCAN all
#end lib_target
