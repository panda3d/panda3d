#define LOCAL_LIBS p3pandabase
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3express
  #define USE_PACKAGES zlib openssl tar
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx $[TARGET]_ext_composite.cxx

  #define SOURCES \
    buffer.I buffer.h \
    ca_bundle_data_src.c \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    compress_string.h \
    config_express.h \
    copy_stream.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    encrypt_string.h \
    error_utils.h \
    export_dtool.h \
    filename_ext.h \
    fileReference.h fileReference.I \
    globPattern_ext.h \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    memoryUsagePointers_ext.h \
    multifile.I multifile.h \
    namable.I \
    namable.h \
    nodePointerTo.h nodePointerTo.I \
    nodePointerToBase.h nodePointerToBase.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    openSSLWrapper.h openSSLWrapper.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pStatCollectorForwardBase.h \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArray_ext.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I profileTimer.h \
    pta_int.h \
    pta_uchar.h pta_double.h pta_float.h \
    pta_stdfloat.h \
    ramfile.I ramfile.h ramfile_ext.h \
    referenceCount.I referenceCount.h \
    subStream.I subStream.h subStreamBuf.h \
    subfileInfo.h subfileInfo.I \
    streamReader_ext.h \
    temporaryFile.h temporaryFile.I \
    threadSafePointerTo.I threadSafePointerTo.h \
    threadSafePointerToBase.I threadSafePointerToBase.h \
    trueClock.I trueClock.h \
    typeHandle_ext.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    vector_uchar.h vector_double.h vector_float.h \
    vector_stdfloat.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I \
    virtualFileMountRamdisk.h virtualFileMountRamdisk.I \
    virtualFileMountSystem.h virtualFileMountSystem.I \
    virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    virtualFileSystem_ext.h \
    weakPointerCallback.I weakPointerCallback.h \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zStream.I zStream.h zStreamBuf.h

  #define INCLUDED_SOURCES  \
    buffer.cxx checksumHashGenerator.cxx \
    compress_string.cxx \
    config_express.cxx \
    copy_stream.cxx \
    datagram.cxx datagramGenerator.cxx \
    datagramIterator.cxx \
    datagramSink.cxx dcast.cxx \
    encrypt_string.cxx \
    error_utils.cxx \
    fileReference.cxx \
    hashGeneratorBase.cxx hashVal.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers_ext.cxx \
    memoryUsagePointers.cxx multifile.cxx \
    namable.cxx \
    nodePointerTo.cxx \
    nodePointerToBase.cxx \
    nodeReferenceCount.cxx \
    openSSLWrapper.cxx \
    ordered_vector.cxx \
    pStatCollectorForwardBase.cxx \
    password_hash.cxx \
    patchfile.cxx \
    pointerTo.cxx \
    pointerToArray.cxx \
    pointerToBase.cxx \
    pointerToVoid.cxx \
    profileTimer.cxx \
    pta_int.cxx \
    pta_uchar.cxx pta_double.cxx pta_float.cxx \
    ramfile_ext.cxx \
    ramfile.cxx \
    referenceCount.cxx \
    streamReader_ext.cxx \
    subStream.cxx subStreamBuf.cxx \
    subfileInfo.cxx \
    temporaryFile.cxx \
    threadSafePointerTo.cxx \
    threadSafePointerToBase.cxx \
    trueClock.cxx \
    typedReferenceCount.cxx \
    vector_uchar.cxx vector_double.cxx vector_float.cxx \
    virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx \
    virtualFileMount.cxx \
    virtualFileMountMultifile.cxx \
    virtualFileMountRamdisk.cxx \
    virtualFileMountSystem.cxx \
    virtualFileSimple.cxx virtualFileSystem.cxx \
    virtualFileSystem_ext.cxx \
    weakPointerCallback.cxx \
    weakPointerTo.cxx \
    weakPointerToBase.cxx \
    weakPointerToVoid.cxx \
    weakReferenceList.cxx \
    windowsRegistry.cxx \
    zStream.cxx zStreamBuf.cxx

  #define INSTALL_HEADERS  \
    buffer.I buffer.h \
    ca_bundle_data_src.c \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    compress_string.h \
    config_express.h \
    copy_stream.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    encrypt_string.h \
    error_utils.h \
    fileReference.h fileReference.I \
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
    nodePointerTo.h nodePointerTo.I \
    nodePointerToBase.h nodePointerToBase.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    openSSLWrapper.h openSSLWrapper.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pStatCollectorForwardBase.h \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    profileTimer.I profileTimer.h \
    pta_int.h \
    pta_uchar.h pta_double.h pta_float.h \
    pta_stdfloat.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    subStream.I subStream.h subStreamBuf.h \
    subfileInfo.h subfileInfo.I \
    temporaryFile.h temporaryFile.I \
    threadSafePointerTo.I threadSafePointerTo.h \
    threadSafePointerToBase.I threadSafePointerToBase.h \
    trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    vector_uchar.h vector_double.h vector_float.h \
    vector_stdfloat.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I \
    virtualFileMountRamdisk.h virtualFileMountRamdisk.I \
    virtualFileMountSystem.h virtualFileMountSystem.I \
    virtualFileSimple.h virtualFileSimple.I \
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
  #define WIN_SYS_LIBS shell32.lib $[WIN_SYS_LIBS]
  #define OSX_SYS_FRAMEWORKS Foundation $[if $[not $[BUILD_IPHONE]],AppKit]

#end lib_target

#begin test_bin_target
  // Not really a "test" program; this program is used to regenerate
  // ca_bundle_data_src.c.

  #define TARGET make_ca_bundle
  #define LOCAL_LIBS $[LOCAL_LIBS] p3express
  #define OTHER_LIBS p3dtoolutil:c p3dtool:m p3pystub

  #define SOURCES \
    make_ca_bundle.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_types
  #define LOCAL_LIBS $[LOCAL_LIBS] p3express
  #define OTHER_LIBS p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m p3pystub

  #define SOURCES \
    test_types.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_ordered_vector

  #define SOURCES \
    test_ordered_vector.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] p3putil
  #define OTHER_LIBS p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m p3pystub

#end test_bin_target


#if $[HAVE_ZLIB]
#begin test_bin_target
  #define TARGET test_zstream
  #define USE_PACKAGES zlib
  #define LOCAL_LIBS $[LOCAL_LIBS] p3express
  #define OTHER_LIBS p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m p3pystub

  #define SOURCES \
    test_zstream.cxx

#end test_bin_target
#endif
