#begin bin_target
  #define TARGET text-stats
  #define LOCAL_LIBS \
    progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c express:c panda:m dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    textMonitor.cxx textMonitor.h textStats.cxx textStats.h

  #define INSTALL_HEADERS \

#end bin_target

