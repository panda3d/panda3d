#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define LOCAL_LIBS \
    net linmath putil express

  #define TARGET pstatclient

  #define SOURCES \
    config_pstats.cxx config_pstats.h pStatClient.I pStatClient.cxx \
    pStatClient.h \
    pStatClientVersion.I pStatClientVersion.cxx pStatClientVersion.h \
    pStatClientControlMessage.cxx \
    pStatClientControlMessage.h \
    pStatCollector.I pStatCollector.h \
    pStatCollectorDef.cxx \
    pStatCollectorDef.h pStatFrameData.I pStatFrameData.cxx \
    pStatFrameData.h pStatProperties.cxx pStatProperties.h \
    pStatServerControlMessage.cxx \
    pStatServerControlMessage.h \
    pStatThread.I pStatThread.h \
    pStatTimer.I pStatTimer.h

  #define INSTALL_HEADERS \
    config_pstats.h pStatClient.I pStatClient.h \
    pStatClientVersion.I pStatClientVersion.h \
    pStatClientControlMessage.h pStatCollector.I pStatCollector.h \
    pStatCollectorDef.h pStatFrameData.I pStatFrameData.h \
    pStatProperties.h \
    pStatServerControlMessage.h pStatThread.I pStatThread.h \
    pStatTimer.I pStatTimer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define LOCAL_LIBS \
    pstatclient
  #define OTHER_LIBS \
    pystub

  #define TARGET test_client

  #define SOURCES \
    test_client.cxx

#end test_bin_target

