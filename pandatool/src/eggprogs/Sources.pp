#begin bin_target
  #define TARGET egg-trans
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c linmath:c putil:c express:c panda:m dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    eggTrans.cxx eggTrans.h

  #define INSTALL_HEADERS \

#end bin_target

