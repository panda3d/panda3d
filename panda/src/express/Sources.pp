#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET express
  #define USE_PACKAGES nspr crypto net zlib
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I atomicAdjust.h \
    atomicAdjust.I atomicAdjustImpl.h atomicAdjustNsprImpl.h \
    atomicAdjustNsprImpl.I \
    bigEndian.h buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h clockObject.I clockObject.h \
    conditionVarDummyImpl.h conditionVarDummyImpl.I conditionVar.h \
    conditionVar.I conditionVarImpl.h conditionVarNsprImpl.h \
    conditionVarNsprImpl.I \
    config_express.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    error_utils.h \
    get_config_path.h \
    globPattern.I globPattern.h \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h indent.I indent.h \
    indirectLess.I indirectLess.h \
    littleEndian.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    mutexDummyImpl.h mutexDummyImpl.I pmutex.h mutexHolder.h \
    mutexHolder.I pmutex.I mutexImpl.h mutexNsprImpl.h mutexNsprImpl.I \
    namable.I \
    namable.h nativeNumericData.I nativeNumericData.h \
    numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    profileTimer.I profileTimer.h \
    pta_uchar.h referenceCount.I referenceCount.h \
    register_type.I register_type.h \
    reversedNumericData.I reversedNumericData.h \
    selectThreadImpl.h \
    streamReader.I streamReader.h streamWriter.I streamWriter.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I threadPriority.h \
    tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I \
    typeHandle.h typedObject.I typedObject.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    typeRegistry.I typeRegistry.h \
    typeRegistryNode.I typeRegistryNode.h \
    unicodeLatinMap.h \
    vector_uchar.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h \
    $[if $[HAVE_CRYPTO], \
       crypto_utils.cxx crypto_utils.h patchfile.I \
       patchfile.cxx patchfile.h ]

  #define INCLUDED_SOURCES  \
    atomicAdjust.cxx atomicAdjustDummyImpl.cxx atomicAdjustNsprImpl.cxx \
    buffer.cxx checksumHashGenerator.cxx clockObject.cxx \
    conditionVar.cxx conditionVarDummyImpl.cxx conditionVarNsprImpl.cxx \
    config_express.cxx datagram.cxx datagramGenerator.cxx \
    datagramIterator.cxx \
    datagramSink.cxx dcast.cxx error_utils.cxx \
    get_config_path.cxx \
    globPattern.cxx \
    hashGeneratorBase.cxx hashVal.cxx indent.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers.cxx multifile.cxx \
    pmutex.cxx mutexHolder.cxx mutexDummyImpl.cxx mutexNsprImpl.cxx \
    namable.cxx \
    nativeNumericData.cxx \
    ordered_vector.cxx \
    profileTimer.cxx \
    pta_uchar.cxx referenceCount.cxx register_type.cxx \
    reversedNumericData.cxx \
    streamReader.cxx streamWriter.cxx \
    stringDecoder.cxx \
    subStream.cxx subStreamBuf.cxx \
    textEncoder.cxx \
    thread.cxx threadDummyImpl.cxx threadNsprImpl.cxx \
    trueClock.cxx typeHandle.cxx \
    typedObject.cxx typedReferenceCount.cxx \
    typeRegistry.cxx typeRegistryNode.cxx \
    unicodeLatinMap.cxx \
    vector_uchar.cxx \
    virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx \
    virtualFileMount.cxx \
    virtualFileMountMultifile.cxx virtualFileMountSystem.cxx \
    virtualFileSimple.cxx virtualFileSystem.cxx \
    windowsRegistry.cxx \
    zStream.cxx zStreamBuf.cxx

  #define INSTALL_HEADERS  \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I atomicAdjust.h \
    atomicAdjust.I atomicAdjustImpl.h atomicAdjustNsprImpl.h \
    atomicAdjustNsprImpl.I \
    bigEndian.h buffer.I buffer.h checksumHashGenerator.I  \
    checksumHashGenerator.h circBuffer.I circBuffer.h clockObject.I \
    clockObject.h \
    conditionVarDummyImpl.h conditionVarDummyImpl.I conditionVar.h \
    conditionVar.I conditionVarImpl.h conditionVarNsprImpl.h \
    conditionVarNsprImpl.I \
    config_express.h datagram.I datagram.h \
    datagramGenerator.I datagramGenerator.h \
    datagramIterator.I datagramIterator.h \
    datagramSink.I datagramSink.h dcast.T dcast.h \
    error_utils.h get_config_path.h \
    globPattern.I globPattern.h \
    hashGeneratorBase.I \
    hashGeneratorBase.h hashVal.I hashVal.h \
    indent.I indent.h \
    indirectLess.I indirectLess.h \
    littleEndian.h memoryInfo.I memoryInfo.h memoryUsage.I \
    memoryUsage.h memoryUsagePointerCounts.I \
    memoryUsagePointerCounts.h memoryUsagePointers.I \
    memoryUsagePointers.h multifile.I multifile.h \
    mutexDummyImpl.h mutexDummyImpl.I pmutex.h mutexHolder.h \
    mutexHolder.I pmutex.I mutexImpl.h mutexNsprImpl.h mutexNsprImpl.I \
    namable.I namable.h \
    nativeNumericData.I nativeNumericData.h numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    patchfile.I patchfile.h pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h profileTimer.I \
    profileTimer.h pta_uchar.h referenceCount.I referenceCount.h \
    register_type.I register_type.h \
    reversedNumericData.I reversedNumericData.h \
    selectThreadImpl.h \
    streamReader.I streamReader.h streamWriter.I streamWriter.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadDummyImpl.h threadDummyImpl.I thread.h thread.I threadImpl.h \
    threadNsprImpl.h threadNsprImpl.I threadPriority.h \
    tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I typeHandle.h \
    typedObject.I typedObject.h typedReferenceCount.I \
    typedReferenceCount.h typedef.h \
    typeRegistry.I typeRegistry.h \
    typeRegistryNode.I typeRegistryNode.h \
    unicodeLatinMap.h \
    vector_uchar.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define IGATESCAN all

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
  #define OTHER_LIBS dtoolutil:c dtool:m pystub

  #define SOURCES \
    test_diners.cxx

#end test_bin_target

