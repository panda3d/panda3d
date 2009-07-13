#define LOCAL_LIBS express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define SELECT_TAU select.tau

#begin lib_target
  #define TARGET pipeline
  #define USE_PACKAGES threads
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    contextSwitch.c contextSwitch.h \
    blockerSimple.h blockerSimple.I \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarFull.h conditionVarFull.I \
    conditionVarFullDebug.h conditionVarFullDebug.I \
    conditionVarFullDirect.h conditionVarFullDirect.I \
    conditionVarFullWin32Impl.h conditionVarFullWin32Impl.I \
    conditionVarImpl.h \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    conditionVarSimpleImpl.h conditionVarSimpleImpl.I \
    conditionVarSpinlockImpl.h conditionVarSpinlockImpl.I \
    config_pipeline.h \
    cycleData.h cycleData.I \
    cycleDataLockedReader.h cycleDataLockedReader.I \
    cycleDataLockedStageReader.h cycleDataLockedStageReader.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    externalThread.h \
    lightMutex.I lightMutex.h \
    lightMutexDirect.h lightMutexDirect.I \
    lightMutexHolder.I lightMutexHolder.h \
    lightReMutex.I lightReMutex.h \
    lightReMutexDirect.h lightReMutexDirect.I \
    lightReMutexHolder.I lightReMutexHolder.h \
    mainThread.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    mutexSimpleImpl.h mutexSimpleImpl.I \
    mutexTrueImpl.h \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    pythonThread.h \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    psemaphore.h psemaphore.I \
    thread.h thread.I threadImpl.h \
    threadDummyImpl.h threadDummyImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadSimpleImpl.h threadSimpleImpl.I  \
    threadSimpleManager.h threadSimpleManager.I  \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h

  #define INCLUDED_SOURCES  \
    conditionVar.cxx \
    conditionVarDebug.cxx \
    conditionVarDirect.cxx \
    conditionVarDummyImpl.cxx \
    conditionVarFull.cxx \
    conditionVarFullDebug.cxx \
    conditionVarFullDirect.cxx \
    conditionVarFullWin32Impl.cxx \
    conditionVarPosixImpl.cxx \
    conditionVarWin32Impl.cxx \
    conditionVarSimpleImpl.cxx \
    conditionVarSpinlockImpl.cxx \
    config_pipeline.cxx \
    cycleData.cxx \
    cycleDataLockedReader.cxx \
    cycleDataLockedStageReader.cxx \
    cycleDataReader.cxx \
    cycleDataStageReader.cxx \
    cycleDataStageWriter.cxx \
    cycleDataWriter.cxx \
    cyclerHolder.cxx \
    externalThread.cxx \
    lightMutex.cxx \
    lightMutexDirect.cxx \
    lightMutexHolder.cxx \
    lightReMutex.cxx \
    lightReMutexDirect.cxx \
    lightReMutexHolder.cxx \
    mainThread.cxx \
    mutexDebug.cxx \
    mutexDirect.cxx \
    mutexHolder.cxx \
    mutexSimpleImpl.cxx \
    pipeline.cxx \
    pipelineCycler.cxx \
    pipelineCyclerDummyImpl.cxx \
    pipelineCyclerTrivialImpl.cxx \
    pipelineCyclerTrueImpl.cxx \
    pmutex.cxx \
    pythonThread.cxx \
    reMutex.cxx \
    reMutexDirect.cxx \
    reMutexHolder.cxx \
    psemaphore.cxx \
    thread.cxx \
    threadDummyImpl.cxx \
    threadPosixImpl.cxx \
    threadSimpleImpl.cxx \
    threadSimpleManager.cxx \
    threadWin32Impl.cxx \
    threadPriority.cxx

  #define INSTALL_HEADERS  \
    contextSwitch.h \
    blockerSimple.h blockerSimple.I \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarFull.h conditionVarFull.I \
    conditionVarFullDebug.h conditionVarFullDebug.I \
    conditionVarFullDirect.h conditionVarFullDirect.I \
    conditionVarFullWin32Impl.h conditionVarFullWin32Impl.I \
    conditionVarImpl.h \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    conditionVarSimpleImpl.h conditionVarSimpleImpl.I \
    conditionVarSpinlockImpl.h conditionVarSpinlockImpl.I \
    config_pipeline.h \
    cycleData.h cycleData.I \
    cycleDataLockedReader.h cycleDataLockedReader.I \
    cycleDataLockedStageReader.h cycleDataLockedStageReader.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    externalThread.h \
    lightMutex.I lightMutex.h \
    lightMutexDirect.h lightMutexDirect.I \
    lightMutexHolder.I lightMutexHolder.h \
    lightReMutex.I lightReMutex.h \
    lightReMutexDirect.h lightReMutexDirect.I \
    lightReMutexHolder.I lightReMutexHolder.h \
    mainThread.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    mutexSimpleImpl.h mutexSimpleImpl.I \
    mutexTrueImpl.h \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    pythonThread.h \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    psemaphore.h psemaphore.I \
    thread.h thread.I threadImpl.h \
    threadDummyImpl.h threadDummyImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadSimpleImpl.h threadSimpleImpl.I \
    threadSimpleManager.h threadSimpleManager.I \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_threaddata
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_threaddata.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_diners
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_diners.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_mutex
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_mutex.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_concurrency
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_concurrency.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_delete
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_delete.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_atomic
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_atomic.cxx

#end test_bin_target



#begin test_bin_target
  #define TARGET test_setjmp
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb:c dconfig:c dtoolbase:c prc:c \
   dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_setjmp.cxx

#end test_bin_target

