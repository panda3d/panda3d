#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    palettizer eggbase progbase
  #define OTHER_LIBS \
    egg:c pgraph:c downloader:c gobj:c lerp:c linmath:c putil:c \
    express:c pnmimage:c pnmimagetypes:c \
    event:c mathutil:c \
    pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS m
  
  #define SOURCES \
     eggPalettize.h eggPalettize.cxx

#end bin_target

