// We won't install egg-trans for now, since the one in $DWDTOOL
// is better.
#begin noinst_bin_target
  #define TARGET egg-trans
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c linmath:c putil:c express:c pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dconfig:c dtool:m pystub

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    eggTrans.cxx eggTrans.h

  #define INSTALL_HEADERS \

#end noinst_bin_target

