#begin bin_target
  #define TARGET egg2x
  #define LOCAL_LIBS xfileegg xfile eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define SOURCES \
    eggToX.cxx eggToX.h

#end bin_target

#begin bin_target
  #define TARGET x2egg
  #define LOCAL_LIBS xfileegg xfile converter eggbase progbase pandatoolbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool pystub \

  #define SOURCES \
    xFileToEgg.cxx xFileToEgg.h

#end bin_target

#begin bin_target
  #define TARGET x-trans
  #define LOCAL_LIBS \
    progbase xfile
  #define OTHER_LIBS \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    xFileTrans.cxx xFileTrans.h

#end bin_target
