#define LOCAL_LIBS p3putil p3express p3pandabase p3pstatclient p3linmath
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3event
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  
  #define SOURCES \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    asyncTaskPause.h asyncTaskPause.I \
    asyncTaskSequence.h asyncTaskSequence.I \
    config_event.h \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    genericAsyncTask.h genericAsyncTask.I \
    pointerEvent.I pointerEvent.h \
    pointerEventList.I pointerEventList.h \
    pythonTask.h pythonTask.I pythonTask.cxx \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h 
    
  #define INCLUDED_SOURCES \
    asyncTask.cxx \
    asyncTaskChain.cxx \
    asyncTaskCollection.cxx \
    asyncTaskManager.cxx \
    asyncTaskPause.cxx \
    asyncTaskSequence.cxx \
    buttonEvent.cxx \
    buttonEventList.cxx \
    genericAsyncTask.cxx \
    pointerEvent.cxx \
    pointerEventList.cxx \
    config_event.cxx event.cxx eventHandler.cxx \ 
    eventParameter.cxx eventQueue.cxx eventReceiver.cxx \
    pt_Event.cxx

  #define INSTALL_HEADERS \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    asyncTaskPause.h asyncTaskPause.I \
    asyncTaskSequence.h asyncTaskSequence.I \
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
  #define LOCAL_LIBS $[LOCAL_LIBS] p3mathutil
  #define OTHER_LIBS \
   p3interrogatedb:c p3dconfig:c p3dtoolbase:c p3prc:c \
   p3dtoolutil:c p3dtool:m p3dtoolconfig:m p3pystub

  #define SOURCES \
    test_task.cxx

#end test_bin_target
