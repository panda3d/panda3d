#define LOCAL_LIBS ipc express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET event
  
  #define SOURCES                                                       \
    config_event.cxx config_event.h \
    event.I event.cxx event.h eventHandler.cxx  \
    eventHandler.h eventParameter.I eventParameter.cxx                  \
    eventParameter.h eventQueue.I eventQueue.cxx eventQueue.h           \
    eventReceiver.cxx eventReceiver.h pt_Event.cxx pt_Event.h           \
    throw_event.I throw_event.h

  #define INSTALL_HEADERS                                       \
    event.I event.h eventHandler.h eventParameter.I eventParameter.h    \
    eventQueue.h eventQueue.I throw_event.h throw_event.I       \
    eventReceiver.h                                             \
    pt_Event.h

  #define IGATESCAN all

#end lib_target
