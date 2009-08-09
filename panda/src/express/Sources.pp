#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET express
  #define USE_PACKAGES zlib openssl tar
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    config_express.h \
    compress_string.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    encrypt_string.h \
    error_utils.h \
    export_dtool.h \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    namable.I \
    namable.h \
    nodePointerToBase.h nodePointerToBase.I \
    nodePointerTo.h nodePointerTo.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I profileTimer.h \
    pStatCollectorForwardBase.h \
    pta_uchar.h pta_float.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadSafePointerTo.I threadSafePointerTo.h \
    threadSafePointerToBase.I threadSafePointerToBase.h \
    trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    unicodeLatinMap.h \
    vector_uchar.h vector_float.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerCallback.I weakPointerCallback.h \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define INCLUDED_SOURCES  \
    buffer.cxx checksumHashGenerator.cxx \
    config_express.cxx \
    compress_string.cxx \
    datagram.cxx datagramGenerator.cxx \
    datagramIterator.cxx \
    datagramSink.cxx dcast.cxx \
    encrypt_string.cxx \
    error_utils.cxx \
    hashGeneratorBase.cxx hashVal.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers.cxx multifile.cxx \
    namable.cxx \
    nodePointerToBase.cxx \
    nodePointerTo.cxx \
    nodeReferenceCount.cxx \
    ordered_vector.cxx \
    password_hash.cxx \
    patchfile.cxx \
    pointerTo.cxx \
    pointerToArray.cxx \
    pointerToBase.cxx \
    pointerToVoid.cxx \
    profileTimer.cxx \
    pStatCollectorForwardBase.cxx \
    pta_uchar.cxx pta_float.cxx \
    ramfile.cxx \
    referenceCount.cxx \
    stringDecoder.cxx \
    subStream.cxx subStreamBuf.cxx \
    textEncoder.cxx \
    threadSafePointerTo.cxx \
    threadSafePointerToBase.cxx \
    trueClock.cxx \
    typedReferenceCount.cxx \
    unicodeLatinMap.cxx \
    vector_uchar.cxx vector_float.cxx \
    virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx \
    virtualFileMount.cxx \
    virtualFileMountMultifile.cxx virtualFileMountSystem.cxx \
    virtualFileSimple.cxx virtualFileSystem.cxx \
    weakPointerCallback.cxx \
    weakPointerTo.cxx \
    weakPointerToBase.cxx \
    weakPointerToVoid.cxx \
    weakReferenceList.cxx \
    windowsRegistry.cxx \
    zStream.cxx zStreamBuf.cxx

  #define INSTALL_HEADERS  \
    buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    config_express.h \
    compress_string.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    encrypt_string.h \
    error_utils.h \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    namable.I \
    namable.h \
    nodePointerToBase.h nodePointerToBase.I \
    nodePointerTo.h nodePointerTo.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I profileTimer.h \
    pStatCollectorForwardBase.h \
    pta_uchar.h pta_float.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    stringDecoder.h stringDecoder.I \
    subStream.I subStream.h subStreamBuf.h \
    textEncoder.h textEncoder.I \
    threadSafePointerTo.I threadSafePointerTo.h \
    threadSafePointerToBase.I threadSafePointerToBase.h \
    trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    unicodeLatinMap.h \
    vector_uchar.h vector_float.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I virtualFileMountSystem.h \
    virtualFileMountSystem.I virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerCallback.I weakPointerCallback.h \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define IGATESCAN all
  #define WIN_SYS_LIBS \
     advapi32.lib ws2_32.lib $[WIN_SYS_LIBS]

  // These libraries and frameworks are used by dtoolutil; we redefine
  // them here so they get into the panda build system.
  #if $[ne $[PLATFORM], FreeBSD]
    #define UNIX_SYS_LIBS dl
  #endif
  #define WIN_SYS_LIBS shell32.lib
  #define OSX_SYS_FRAMEWORKS Foundation $[if $[not $[BUILD_IPHONE]],AppKit]

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
