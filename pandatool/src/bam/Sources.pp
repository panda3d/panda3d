#begin bin_target
  #define TARGET bam-info
  #define LOCAL_LIBS \
    progbase
  #define OTHER_LIBS \
    recorder:c parametrics:c collide:c chan:c char:c \
    dgraph:c downloader:c egg:c \
    pnmimagetypes:c pnmimage:c pgraph:c gobj:c putil:c \
    lerp:c mathutil:c linmath:c event:c express:c \
    pandaegg:m panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    bamInfo.cxx bamInfo.h

  #define INSTALL_HEADERS
#end bin_target

#begin bin_target
  #define TARGET egg2bam
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    builder:c collide:c chan:c char:c display:c downloader:c \
    dgraph:c egg2pg:c egg:c event:c express:c \
    grutil:c gobj:c gsgbase:c lerp:c linmath:c mathutil:c \
    pgraph:c parametrics:c pnmimagetypes:c pnmimage:c putil:c \
    text:c tform:c \
    pandaegg:m panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    eggToBam.cxx eggToBam.h
#end bin_target


#begin bin_target
  #define TARGET bam2egg
  #define LOCAL_LIBS \
    converter eggbase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pgraph:c parametrics:c collide:c chan:c char:c \
    downloader:c mathutil:c \
    gobj:c lerp:c pnmimagetypes:c pnmimage:c pstatclient:c \
    putil:c linmath:c event:c express:c panda:m pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    bamToEgg.cxx bamToEgg.h
#end bin_target
