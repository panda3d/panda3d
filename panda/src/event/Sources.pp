#define LOCAL_LIBS putil express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET event
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx
  
  #define SOURCES \
    config_event.h \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h 
    
  #define INCLUDED_SOURCES \
    buttonEvent.cxx \
    buttonEventList.cxx \
    config_event.cxx event.cxx eventHandler.cxx \ 
    eventParameter.cxx eventQueue.cxx eventReceiver.cxx \
    pt_Event.cxx

  #define INSTALL_HEADERS \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h 

  #define IGATESCAN all

#end lib_target
