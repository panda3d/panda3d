#define LOCAL_LIBS putil express pandabase pstatclient linmath
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET event
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  
  #define SOURCES \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    config_event.h \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    genericAsyncTask.h genericAsyncTask.I \
    pointerEvent.I pointerEvent.h \
    pointerEventList.I pointerEventList.h \
    pythonTask.h pythonTask.I \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h 
    
  #define INCLUDED_SOURCES \
    asyncTask.cxx \
    asyncTaskChain.cxx \
    asyncTaskCollection.cxx \
    asyncTaskManager.cxx \
    buttonEvent.cxx \
    buttonEventList.cxx \
    genericAsyncTask.cxx \
    pointerEvent.cxx \
    pointerEventList.cxx \
    pythonTask.cxx \
    config_event.cxx event.cxx eventHandler.cxx \ 
    eventParameter.cxx eventQueue.cxx eventReceiver.cxx \
    pt_Event.cxx

  #define INSTALL_HEADERS \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    genericAsyncTask.h genericAsyncTask.I \
    pointerEvent.I pointerEvent.h \
    pointerEventList.I pointerEventList.h \
    pythonTask.h pythonTask.I \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h 

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_task
  #define LOCAL_LIBS $[LOCAL_LIBS] mathutil
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_task.cxx

#end test_bin_target
