#define OTHER_LIBS \
  egg:c pandaegg:m \
  pipeline:c mathutil:c linmath:c putil:c \
  event:c pnmimage:c \
  panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS xfileegg xfile eggbase progbase pandatoolbase

  #define SOURCES \
    eggToX.cxx eggToX.h

#end bin_target

#begin bin_target
  #define TARGET x2egg
  #define LOCAL_LIBS xfileegg xfile converter eggbase progbase pandatoolbase

  #define SOURCES \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target

#begin bin_target
  #define TARGET x-trans
  #define LOCAL_LIBS \
    progbase xfile

  #define SOURCES \
    xFileTrans.cxx xFileTrans.h

#end bin_target
