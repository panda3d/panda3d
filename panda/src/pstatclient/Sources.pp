#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c 

#begin lib_target
  #define LOCAL_LIBS \
    p3net p3putil p3express

  #define TARGET p3pstatclient
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx   

  #define SOURCES \
     config_pstats.h pStatClient.I pStatClient.h \
     pStatClientImpl.I pStatClientImpl.h \
     pStatClientVersion.I  \
     pStatClientVersion.h pStatClientControlMessage.h  \
     pStatCollector.I pStatCollector.h pStatCollectorDef.h  \
     pStatCollectorForward.I pStatCollectorForward.h \
     pStatFrameData.I pStatFrameData.h pStatProperties.h  \
     pStatServerControlMessage.h pStatThread.I pStatThread.h  \
     pStatTimer.I pStatTimer.h

  #define INCLUDED_SOURCES  \
     config_pstats.cxx pStatClient.cxx pStatClientImpl.cxx \
     pStatClientVersion.cxx  \
     pStatClientControlMessage.cxx \
     pStatCollector.cxx \
     pStatCollectorDef.cxx  \
     pStatCollectorForward.cxx \
     pStatFrameData.cxx pStatProperties.cxx  \
     pStatServerControlMessage.cxx \
     pStatThread.cxx

  #define INSTALL_HEADERS \
    config_pstats.h pStatClient.I pStatClient.h \
    pStatClientImpl.I pStatClientImpl.h \
    pStatClientVersion.I pStatClientVersion.h \
    pStatClientControlMessage.h pStatCollector.I pStatCollector.h \
    pStatCollectorDef.h \
    pStatCollectorForward.I pStatCollectorForward.h \
    pStatFrameData.I pStatFrameData.h \
    pStatProperties.h \
    pStatServerControlMessage.h pStatThread.I pStatThread.h \
    pStatTimer.I pStatTimer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define LOCAL_LIBS \
    p3pstatclient 
  #define OTHER_LIBS \
    $[OTHER_LIBS] p3pystub

  #define TARGET test_client

  #define SOURCES \
    test_client.cxx

#end test_bin_target

