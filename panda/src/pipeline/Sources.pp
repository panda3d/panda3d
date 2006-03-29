#define LOCAL_LIBS express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET pipeline
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarImpl.h \
    conditionVarNsprImpl.h conditionVarNsprImpl.I \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    config_pipeline.h \
    cycleData.h cycleData.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    externalThread.h \
    mainThread.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h

  #define INCLUDED_SOURCES  \
    conditionVar.cxx \
    conditionVarDebug.cxx \
    conditionVarDirect.cxx \
    conditionVarDummyImpl.cxx \
    conditionVarNsprImpl.cxx \
    conditionVarPosixImpl.cxx \
    conditionVarWin32Impl.cxx \
    config_pipeline.cxx \
    cycleData.cxx \
    cycleDataReader.cxx \
    cycleDataStageReader.cxx \
    cycleDataStageWriter.cxx \
    cycleDataWriter.cxx \
    cyclerHolder.cxx \
    externalThread.cxx \
    mainThread.cxx \
    mutexDebug.cxx \
    mutexDirect.cxx \
    mutexHolder.cxx \
    pipeline.cxx \
    pipelineCycler.cxx \
    pipelineCyclerDummyImpl.cxx \
    pipelineCyclerTrivialImpl.cxx \
    pipelineCyclerTrueImpl.cxx \
    pmutex.cxx \
    reMutex.cxx \
    reMutexDirect.cxx \
    reMutexHolder.cxx \
    thread.cxx threadDummyImpl.cxx \
    threadNsprImpl.cxx \
    threadPosixImpl.cxx \
    threadWin32Impl.cxx

  #define INSTALL_HEADERS  \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarImpl.h \
    conditionVarNsprImpl.h conditionVarNsprImpl.I \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    config_pipeline.h \
    cycleData.h cycleData.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    externalThread.h \
    mainThread.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_threaddata
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m pystub

  #define SOURCES \
    test_threaddata.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_diners
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m dtoolconfig:m pystub

  #define SOURCES \
    test_diners.cxx

#end test_bin_target

