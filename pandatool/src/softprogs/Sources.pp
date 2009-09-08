#begin bin_target
  #define TARGET softcvs
  #define LOCAL_LIBS progbase
  #define USE_PACKAGES openssl

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    event:c mathutil:c pnmimage:c putil:c \
    linmath:c pipeline:c event:c \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    softCVS.cxx softCVS.h softFilename.cxx softFilename.h

#end bin_target
