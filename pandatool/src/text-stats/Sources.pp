#define BUILD_DIRECTORY $[HAVE_NET]

#begin bin_target
  #define TARGET text-stats
  #define LOCAL_LIBS \
    progbase pstatserver
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pstatclient:c linmath:c putil:c express:c panda:m pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    textMonitor.cxx textMonitor.h textStats.cxx textStats.h

  #define INSTALL_HEADERS 

#end bin_target

