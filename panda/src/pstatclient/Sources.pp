#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define LOCAL_LIBS \
    net linmath putil express

  #define TARGET pstatclient
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx   

  #define SOURCES \
     config_pstats.h pStatClient.I pStatClient.h pStatClientVersion.I  \
     pStatClientVersion.h pStatClientControlMessage.h  \
     pStatCollector.I pStatCollector.h pStatCollectorDef.h  \
     pStatFrameData.I pStatFrameData.h pStatProperties.h  \
     pStatServerControlMessage.h pStatThread.I pStatThread.h  \
     pStatTimer.I pStatTimer.h

  #define INCLUDED_SOURCES  \
     config_pstats.cxx pStatClient.cxx pStatClientVersion.cxx  \
     pStatClientControlMessage.cxx pStatCollectorDef.cxx  \
     pStatFrameData.cxx pStatProperties.cxx  \
     pStatServerControlMessage.cxx 

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
    $[OTHER_LIBS] pystub

  #define TARGET test_client

  #define SOURCES \
    test_client.cxx

#end test_bin_target

