set(P3PIPELINE_HEADERS
  contextSwitch.h
  blockerSimple.h blockerSimple.I
  conditionVar.h conditionVar.I
  conditionVarDebug.h conditionVarDebug.I
  conditionVarDirect.h conditionVarDirect.I
  conditionVarDummyImpl.h conditionVarDummyImpl.I
  conditionVarFull.h
  conditionVarImpl.h
  conditionVarSimpleImpl.h conditionVarSimpleImpl.I
  conditionVarSpinlockImpl.h conditionVarSpinlockImpl.I
  conditionVarPosixImpl.h conditionVarPosixImpl.I
  config_pipeline.h
  cycleData.h cycleData.I
  cycleDataLockedReader.h cycleDataLockedReader.I
  cycleDataLockedStageReader.h cycleDataLockedStageReader.I
  cycleDataReader.h cycleDataReader.I
  cycleDataStageReader.h cycleDataStageReader.I
  cycleDataStageWriter.h cycleDataStageWriter.I
  cycleDataWriter.h cycleDataWriter.I
  cyclerHolder.h cyclerHolder.I
  externalThread.h
  genericThread.h genericThread.I
  lightMutex.I lightMutex.h
  lightMutexDirect.h lightMutexDirect.I
  lightMutexHolder.I lightMutexHolder.h
  lightReMutex.I lightReMutex.h
  lightReMutexDirect.h lightReMutexDirect.I
  lightReMutexHolder.I lightReMutexHolder.h
  mainThread.h
  mutexDebug.h mutexDebug.I
  mutexDirect.h mutexDirect.I
  mutexHolder.h mutexHolder.I
  mutexSimpleImpl.h mutexSimpleImpl.I
  mutexTrueImpl.h
  pipeline.h pipeline.I
  pipelineCycler.h pipelineCycler.I
  pipelineCyclerLinks.h pipelineCyclerLinks.I
  pipelineCyclerBase.h
  pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I
  pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I
  pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I
  pmutex.h pmutex.I
  reMutex.I reMutex.h
  reMutexDirect.h reMutexDirect.I
  reMutexHolder.h reMutexHolder.I
  reMutexSpinlockImpl.h reMutexSpinlockImpl.I
  psemaphore.h psemaphore.I
  thread.h thread.I threadImpl.h
  threadDummyImpl.h threadDummyImpl.I
  threadSimpleImpl.h threadSimpleImpl.I
  threadPosixImpl.h threadPosixImpl.I
  threadSimpleManager.h threadSimpleManager.I
  threadPriority.h
)

set(P3PIPELINE_SOURCES
  contextSwitch.c
  conditionVar.cxx
  conditionVarDebug.cxx
  conditionVarDirect.cxx
  conditionVarDummyImpl.cxx
  conditionVarSimpleImpl.cxx
  conditionVarSpinlockImpl.cxx
  conditionVarPosixImpl.cxx
  config_pipeline.cxx
  cycleData.cxx
  cycleDataLockedReader.cxx
  cycleDataLockedStageReader.cxx
  cycleDataReader.cxx
  cycleDataStageReader.cxx
  cycleDataStageWriter.cxx
  cycleDataWriter.cxx
  cyclerHolder.cxx
  externalThread.cxx
  genericThread.cxx
  lightMutex.cxx
  lightMutexDirect.cxx
  lightMutexHolder.cxx
  lightReMutex.cxx
  lightReMutexDirect.cxx
  lightReMutexHolder.cxx
  mainThread.cxx
  mutexDebug.cxx
  mutexDirect.cxx
  mutexHolder.cxx
  mutexSimpleImpl.cxx
  pipeline.cxx
  pipelineCycler.cxx
  pipelineCyclerDummyImpl.cxx
  pipelineCyclerTrivialImpl.cxx
  pipelineCyclerTrueImpl.cxx
  pmutex.cxx
  reMutex.cxx
  reMutexDirect.cxx
  reMutexHolder.cxx
  reMutexSpinlockImpl.cxx
  psemaphore.cxx
  thread.cxx
  threadDummyImpl.cxx
  threadPosixImpl.cxx
  threadSimpleImpl.cxx
  threadSimpleManager.cxx
  threadPriority.cxx
)

if(WIN32)
  list(APPEND P3PIPELINE_HEADERS
    conditionVarWin32Impl.h conditionVarWin32Impl.I
    threadWin32Impl.h threadWin32Impl.I)
  list(APPEND P3PIPELINE_SOURCES
    conditionVarWin32Impl.cxx
    threadWin32Impl.cxx)
endif()

set(P3PIPELINE_IGATEEXT
  pmutex_ext.h
  pythonThread.cxx
  pythonThread.h
  reMutex_ext.h
)

composite_sources(p3pipeline P3PIPELINE_SOURCES)
add_component_library(p3pipeline SYMBOL BUILDING_PANDA_PIPELINE
  ${P3PIPELINE_HEADERS} ${P3PIPELINE_SOURCES})
target_link_libraries(p3pipeline pandaexpress
  PKG::THREADS)
target_interrogate(p3pipeline ALL EXTENSIONS ${P3PIPELINE_IGATEEXT})

if(PHAVE_UCONTEXT_H)
  target_compile_definitions(p3pipeline PRIVATE PHAVE_UCONTEXT_H)
endif()

if(OS_SIMPLE_THREADS)
  target_compile_definitions(p3pipeline PRIVATE OS_SIMPLE_THREADS)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3pipeline
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PIPELINE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
