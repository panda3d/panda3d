#define OTHER_LIBS \
  egg:c pandaegg:m \
  linmath:c mathutil:c pipeline:c event:c \
  pnmimagetypes:c pnmimage:c putil:c \
  panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET lwo2egg
  #define LOCAL_LIBS lwo lwoegg eggbase progbase

  #define SOURCES \
    lwoToEgg.cxx lwoToEgg.h

#end bin_target

#begin bin_target
  #define TARGET lwo-scan
  #define LOCAL_LIBS lwo progbase

  #define SOURCES \
    lwoScan.cxx lwoScan.h

#end bin_target
