#begin bin_target
  #define TARGET softcvs
  #define LOCAL_LIBS progbase

  #define OTHER_LIBS \
    express:c pandaexpress:m \
    dtoolutil:c dconfig:c dtool:m pystub

  #define SOURCES \
    softCVS.cxx softCVS.h softFilename.cxx softFilename.h

#end bin_target
