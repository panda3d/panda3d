#define LOCAL_LIBS \
  eggbase progbase
#define OTHER_LIBS \
  egg:c linmath:c putil:c express:c pandaegg:m panda:m pandaexpress:m \
  dtoolutil:c dconfig:c dtool:m pystub


#begin bin_target
  #define TARGET egg-trans

  #define SOURCES \
    eggTrans.cxx eggTrans.h

#end bin_target

#begin bin_target
  #define TARGET egg-texture-cards

  #define SOURCES \
    eggTextureCards.cxx eggTextureCards.h

#end bin_target

#begin noinst_bin_target
  #define TARGET egg-topstrip

  #define SOURCES \
    eggTopstrip.cxx eggTopstrip.h

#end noinst_bin_target
