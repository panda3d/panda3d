// We won't install image-trans for now, since the one in $DWDTOOL
// is better.
#begin noinst_bin_target
  #define TARGET image-trans
  #define LOCAL_LIBS \
    imagebase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pnmimagetypes:c pnmimage:c putil:c express:c panda:m \
    pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    imageTrans.cxx imageTrans.h

  #define INSTALL_HEADERS \

#end noinst_bin_target

