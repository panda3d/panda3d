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
add_component_library(p3event NOINIT SYMBOL BUILDING_PANDA_EVENT
  ${P3EVENT_HEADERS} ${P3EVENT_SOURCES})
target_link_libraries(p3event p3linmath p3pstatclient)
target_interrogate(p3event ALL EXTENSIONS ${P3EVENT_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3event
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3EVENT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
