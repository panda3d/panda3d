#define LOCAL_LIBS \
  progbase
#define OTHER_LIBS \
  pnmimage:c linmath:c \
  putil:c panda:m \
  express:c pandaexpress:m \
  dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET bin2c

  #define SOURCES \
    binToC.cxx binToC.h

#end bin_target
