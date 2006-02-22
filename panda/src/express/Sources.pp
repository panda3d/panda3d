#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET express
  #define USE_PACKAGES nspr net zlib openssl
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I atomicAdjust.h \
    atomicAdjust.I atomicAdjustImpl.h \
    atomicAdjustNsprImpl.h atomicAdjustNsprImpl.I \
    atomicAdjustPosixImpl.h atomicAdjustPosixImpl.I \
    atomicAdjustWin32Impl.h atomicAdjustWin32Impl.I \
    bigEndian.h buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h clockObject.I clockObject.h \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarImpl.h \
    conditionVarNsprImpl.h conditionVarNsprImpl.I \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    config_express.h \
    cycleData.h cycleData.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I \
    error_utils.h \
    externalThread.h \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    littleEndian.h \
    mainThread.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    namable.I \
    namable.h nativeNumericData.I nativeNumericData.h \
    numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    password_hash.h \
    patchfile.I patchfile.h \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I profileTimer.h \
    pta_uchar.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    reversedNumericData.I reversedNumericData.h \
    streamReader.I streamReader.h streamWriter.I streamWriter.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h \
    tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    unicodeLatinMap.h \
    vector_uchar.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define INCLUDED_SOURCES  \
    atomicAdjust.cxx atomicAdjustDummyImpl.cxx \
    atomicAdjustNsprImpl.cxx \
    atomicAdjustPosixImpl.cxx \
    atomicAdjustWin32Impl.cxx \
    buffer.cxx checksumHashGenerator.cxx clockObject.cxx \
    conditionVar.cxx \
    conditionVarDebug.cxx \
    conditionVarDirect.cxx \
    conditionVarDummyImpl.cxx \
    conditionVarNsprImpl.cxx \
    conditionVarPosixImpl.cxx \
    conditionVarWin32Impl.cxx \
    config_express.cxx \
    cycleData.cxx \
    cycleDataReader.cxx \
    cycleDataStageReader.cxx \
    cycleDataStageWriter.cxx \
    cycleDataWriter.cxx \
    cyclerHolder.cxx \
    datagram.cxx datagramGenerator.cxx \
    datagramIterator.cxx \
    datagramSink.cxx dcast.cxx \
    encryptStreamBuf.cxx encryptStream.cxx \
    error_utils.cxx \
    externalThread.cxx \
    hashGeneratorBase.cxx hashVal.cxx \
    mainThread.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers.cxx multifile.cxx \
    mutexDebug.cxx \
    mutexDirect.cxx \
    mutexHolder.cxx \
    pipeline.cxx \
    pipelineCycler.cxx \
    pipelineCyclerDummyImpl.cxx \
    pipelineCyclerTrivialImpl.cxx \
    pipelineCyclerTrueImpl.cxx \
    pmutex.cxx \
    namable.cxx \
    nativeNumericData.cxx \
    ordered_vector.cxx \
    password_hash.cxx \
    patchfile.cxx \
    pointerTo.cxx \
    pointerToArray.cxx \
    pointerToBase.cxx \
    pointerToVoid.cxx \
    profileTimer.cxx \
    pta_uchar.cxx \
    ramfile.cxx \
    referenceCount.cxx \
    reMutex.cxx \
    reMutexDirect.cxx \
    reMutexHolder.cxx \
    reversedNumericData.cxx \
    streamReader.cxx streamWriter.cxx \
    stringDecoder.cxx \
    subStream.cxx subStreamBuf.cxx \
    textEncoder.cxx \
    thread.cxx threadDummyImpl.cxx \
    threadNsprImpl.cxx \
    threadPosixImpl.cxx \
    threadWin32Impl.cxx \
    trueClock.cxx \
    typedReferenceCount.cxx \
    unicodeLatinMap.cxx \
    vector_uchar.cxx \
    virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx \
    virtualFileMount.cxx \
    virtualFileMountMultifile.cxx virtualFileMountSystem.cxx \
    virtualFileSimple.cxx virtualFileSystem.cxx \
    weakPointerTo.cxx \
    weakPointerToBase.cxx \
    weakPointerToVoid.cxx \
    weakReferenceList.cxx \
    windowsRegistry.cxx \
    zStream.cxx zStreamBuf.cxx

  #define INSTALL_HEADERS  \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I atomicAdjust.h \
    atomicAdjust.I atomicAdjustImpl.h \
    atomicAdjustNsprImpl.h atomicAdjustNsprImpl.I \
    atomicAdjustPosixImpl.h atomicAdjustPosixImpl.I \
    atomicAdjustWin32Impl.h atomicAdjustWin32Impl.I \
    bigEndian.h buffer.I buffer.h checksumHashGenerator.I  \
    checksumHashGenerator.h circBuffer.I circBuffer.h clockObject.I \
    clockObject.h \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarImpl.h \
    conditionVarNsprImpl.h conditionVarNsprImpl.I \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    conditionVarWin32Impl.h conditionVarWin32Impl.I \
    config_express.h \
    cycleData.h cycleData.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    datagram.I datagram.h \
    datagramGenerator.I datagramGenerator.h \
    datagramIterator.I datagramIterator.h \
    datagramSink.I datagramSink.h dcast.T dcast.h \
    encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I \
    error_utils.h  \
    externalThread.h \
    hashGeneratorBase.I \
    hashGeneratorBase.h hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    littleEndian.h \
    mainThread.h \
    memoryInfo.I memoryInfo.h memoryUsage.I \
    memoryUsage.h memoryUsagePointerCounts.I \
    memoryUsagePointerCounts.h memoryUsagePointers.I \
    memoryUsagePointers.h multifile.I multifile.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    namable.I namable.h \
    nativeNumericData.I nativeNumericData.h numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    password_hash.h \
    patchfile.I patchfile.h \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I \
    profileTimer.h pta_uchar.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    reversedNumericData.I reversedNumericData.h \
    streamReader.I streamReader.h streamWriter.I streamWriter.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadWin32Impl.h threadWin32Impl.I \
    threadPriority.h \
    tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h \
    typedReferenceCount.I \
    typedReferenceCount.h typedef.h \
    unicodeLatinMap.h \
    vector_uchar.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define IGATESCAN all
  #define WIN_SYS_LIBS \
     advapi32.lib ws2_32.lib $[WIN_SYS_LIBS]

#end lib_target

#begin test_bin_target
  #define TARGET test_types
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS pystub

  #define SOURCES \
    test_types.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_ordered_vector

  #define SOURCES \
    test_ordered_vector.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target


#if $[HAVE_ZLIB]
#begin test_bin_target
  #define TARGET test_zstream
  #define USE_PACKAGES zlib
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m pystub

  #define SOURCES \
    test_zstream.cxx

#end test_bin_target
#endif


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

