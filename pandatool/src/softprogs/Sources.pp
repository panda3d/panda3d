#begin bin_target
  #define TARGET softcvs
  #define LOCAL_LIBS progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pnmimage:c putil:c \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    softCVS.cxx softCVS.h softFilename.cxx softFilename.h

#end bin_target
