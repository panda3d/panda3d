#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pstatclient
  #define LOCAL_LIBS \
    net linmath putil

  #define SOURCES \
    config_pstats.cxx config_pstats.h pStatClient.I pStatClient.cxx \
    pStatClient.h pStatClientControlMessage.cxx \
    pStatClientControlMessage.h pStatCollectorDef.cxx \
    pStatCollectorDef.h pStatFrameData.I pStatFrameData.cxx \
    pStatFrameData.h pStatServerControlMessage.cxx \
    pStatServerControlMessage.h

  #define INSTALL_HEADERS \
    config_pstats.h pStatClient.I pStatClient.h \
    pStatClientControlMessage.h pStatCollector.I pStatCollector.h \
    pStatCollectorDef.h pStatFrameData.I pStatFrameData.h \
    pStatServerControlMessage.h pStatThread.I pStatThread.h \
    pStatTimer.I pStatTimer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_client

  #define SOURCES \
    test_client.cxx

#end test_bin_target

