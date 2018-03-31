set(P3EVENT_HEADERS
    asyncFuture.h asyncFuture.I
    asyncTask.h asyncTask.I
    asyncTaskChain.h asyncTaskChain.I
    asyncTaskCollection.h asyncTaskCollection.I
    asyncTaskManager.h asyncTaskManager.I
    asyncTaskPause.h asyncTaskPause.I
    asyncTaskSequence.h asyncTaskSequence.I
    config_event.h
    buttonEvent.I buttonEvent.h
    buttonEventList.I buttonEventList.h
    genericAsyncTask.h genericAsyncTask.I
    pointerEvent.I pointerEvent.h
    pointerEventList.I pointerEventList.h
    event.I event.h eventHandler.h eventHandler.I
    eventParameter.I eventParameter.h
    eventQueue.I eventQueue.h eventReceiver.h
    pt_Event.h throw_event.I throw_event.h 
)

set(P3EVENT_SOURCES
    asyncFuture.cxx
    asyncTask.cxx
    asyncTaskChain.cxx
    asyncTaskCollection.cxx
    asyncTaskManager.cxx
    asyncTaskPause.cxx
    asyncTaskSequence.cxx
    buttonEvent.cxx
    buttonEventList.cxx
    genericAsyncTask.cxx
    pointerEvent.cxx
    pointerEventList.cxx
    config_event.cxx event.cxx eventHandler.cxx 
    eventParameter.cxx eventQueue.cxx eventReceiver.cxx
    pt_Event.cxx
)

set(P3EVENT_IGATEEXT
  asyncFuture_ext.cxx
  asyncFuture_ext.h
  pythonTask.cxx
  pythonTask.h
  pythonTask.I
)

composite_sources(p3event P3EVENT_SOURCES)
add_library(p3event ${P3EVENT_HEADERS} ${P3EVENT_SOURCES})
set_target_properties(p3event PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_EVENT)
target_link_libraries(p3event p3linmath p3pstatclient)
target_interrogate(p3event ALL EXTENSIONS ${P3EVENT_IGATEEXT})

install(TARGETS p3event DESTINATION lib)
install(FILES ${P3EVENT_HEADERS} DESTINATION include/panda3d)

#begin test_bin_target
  #define TARGET test_task
  #define LOCAL_LIBS $[LOCAL_LIBS] p3mathutil
  #define OTHER_LIBS \
#   p3interrogatedb:c p3dconfig:c p3dtoolbase:c p3prc:c \
#   p3dtoolutil:c p3dtool:m p3dtoolconfig:m p3pystub

  #define SOURCES \
#    test_task.cxx

#end test_bin_target
