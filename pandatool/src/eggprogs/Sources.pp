#define LOCAL_LIBS \
  eggbase progbase
#define OTHER_LIBS \
  egg:c linmath:c putil:c express:c pandaegg:m panda:m pandaexpress:m \
  dtoolutil:c dconfig:c dtool:m pystub


// We won't install egg-trans for now, since the one in $DWDTOOL
// is better.
#begin noinst_bin_target
  #define TARGET egg-trans

  #define SOURCES \
    eggTrans.cxx eggTrans.h

#end noinst_bin_target
