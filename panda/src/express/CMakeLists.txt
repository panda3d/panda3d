set(P3EXPRESS_HEADERS
  buffer.I buffer.h
  checksumHashGenerator.I checksumHashGenerator.h circBuffer.I
  circBuffer.h
  compress_string.h
  config_express.h
  copy_stream.h
  datagram.I datagram.h datagramGenerator.I
  datagramGenerator.h
  datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h
  dcast.T dcast.h
  encrypt_string.h
  error_utils.h
  export_dtool.h
  fileReference.h fileReference.I
  hashGeneratorBase.I hashGeneratorBase.h
  hashVal.I hashVal.h
  indirectLess.I indirectLess.h
  memoryInfo.I memoryInfo.h
  memoryUsage.I memoryUsage.h
  memoryUsagePointerCounts.I memoryUsagePointerCounts.h
  memoryUsagePointers.I memoryUsagePointers.h
  multifile.I multifile.h
  namable.I
  namable.h
  nodePointerTo.h nodePointerTo.I
  nodePointerToBase.h nodePointerToBase.I
  nodeReferenceCount.h nodeReferenceCount.I
  openSSLWrapper.h openSSLWrapper.I
  ordered_vector.h ordered_vector.I ordered_vector.T
  pStatCollectorForwardBase.h
  password_hash.h
  patchfile.I patchfile.h
  pointerTo.I pointerTo.h
  pointerToArray.I pointerToArray.h
  pointerToArrayBase.I pointerToArrayBase.h
  pointerToBase.I pointerToBase.h
  pointerToVoid.I pointerToVoid.h
  pta_int.h
  pta_uchar.h pta_double.h pta_float.h
  pta_stdfloat.h
  ramfile.I ramfile.h
  referenceCount.I referenceCount.h
  stringStreamBuf.I stringStreamBuf.h
  stringStream.I stringStream.h
  subStream.I subStream.h subStreamBuf.h
  subfileInfo.h subfileInfo.I
  temporaryFile.h temporaryFile.I
  trueClock.I trueClock.h
  typedReferenceCount.I typedReferenceCount.h
  virtualFileList.I virtualFileList.h
  virtualFileComposite.h virtualFileComposite.I
  virtualFile.h virtualFile.I
  virtualFileMount.h virtualFileMount.I
  virtualFileMountAndroidAsset.h virtualFileMountAndroidAsset.I
  virtualFileMountMultifile.h virtualFileMountMultifile.I
  virtualFileMountRamdisk.h virtualFileMountRamdisk.I
  virtualFileMountSystem.h virtualFileMountSystem.I
  virtualFileMountZip.h virtualFileMountZip.I
  virtualFileSimple.h virtualFileSimple.I
  virtualFileSystem.h virtualFileSystem.I
  weakPointerCallback.I weakPointerCallback.h
  weakPointerTo.I weakPointerTo.h
  weakPointerToBase.I weakPointerToBase.h
  weakPointerToVoid.I weakPointerToVoid.h
  weakReferenceList.I weakReferenceList.h
  windowsRegistry.h
  zipArchive.I zipArchive.h
  zStream.I zStream.h zStreamBuf.h
)

set(P3EXPRESS_SOURCES
  buffer.cxx checksumHashGenerator.cxx
  compress_string.cxx
  config_express.cxx
  copy_stream.cxx
  datagram.cxx datagramGenerator.cxx
  datagramIterator.cxx
  datagramSink.cxx dcast.cxx
  encrypt_string.cxx
  error_utils.cxx
  fileReference.cxx
  hashGeneratorBase.cxx hashVal.cxx
  memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx
  memoryUsagePointers.cxx multifile.cxx
  namable.cxx
  nodePointerTo.cxx
  nodePointerToBase.cxx
  nodeReferenceCount.cxx
  openSSLWrapper.cxx
  ordered_vector.cxx
  pStatCollectorForwardBase.cxx
  password_hash.cxx
  patchfile.cxx
  pointerTo.cxx
  pointerToArray.cxx
  pointerToBase.cxx
  pointerToVoid.cxx
  pta_int.cxx
  pta_uchar.cxx pta_double.cxx pta_float.cxx
  ramfile.cxx
  referenceCount.cxx
  stringStreamBuf.cxx
  stringStream.cxx
  subStream.cxx subStreamBuf.cxx
  subfileInfo.cxx
  temporaryFile.cxx
  trueClock.cxx
  typedReferenceCount.cxx
  virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx
  virtualFileMount.cxx
  virtualFileMountMultifile.cxx
  virtualFileMountRamdisk.cxx
  virtualFileMountSystem.cxx
  virtualFileMountZip.cxx
  virtualFileSimple.cxx virtualFileSystem.cxx
  weakPointerCallback.cxx
  weakPointerTo.cxx
  weakPointerToBase.cxx
  weakPointerToVoid.cxx
  weakReferenceList.cxx
  windowsRegistry.cxx
  zipArchive.cxx
  zStream.cxx zStreamBuf.cxx
)

if(ANDROID)
  list(APPEND P3EXPRESS_SOURCES virtualFileMountAndroidAsset.cxx)
endif()

set(P3EXPRESS_IGATEEXT
  datagram_ext.h
  memoryUsagePointers_ext.cxx
  memoryUsagePointers_ext.h
  pointerToArray_ext.h
  ramfile_ext.cxx
  ramfile_ext.h
  stringStream_ext.cxx
  stringStream_ext.h
  virtualFileSystem_ext.cxx
  virtualFileSystem_ext.h
  virtualFile_ext.cxx
  virtualFile_ext.h
  multifile_ext.h
  multifile_ext.I
)

composite_sources(p3express P3EXPRESS_SOURCES)
add_component_library(p3express SYMBOL BUILDING_PANDA_EXPRESS
  ${P3EXPRESS_SOURCES} ${P3EXPRESS_HEADERS})
target_link_libraries(p3express p3pandabase p3dconfig p3prc p3dtool
  PKG::ZLIB PKG::OPENSSL)
target_interrogate(p3express ALL EXTENSIONS ${P3EXPRESS_IGATEEXT})

if(REPORT_OPENSSL_ERRORS)
  target_compile_definitions(p3express PRIVATE REPORT_OPENSSL_ERRORS)
endif()
if(REPORT_OPENSSL_ERRORS_Debug)
  target_compile_definitions(p3express PRIVATE $<$<CONFIG:Debug>:REPORT_OPENSSL_ERRORS>)
endif()

if(GETTIMEOFDAY_ONE_PARAM)
  target_compile_definitions(p3express PRIVATE GETTIMEOFDAY_ONE_PARAM)
endif()

if(WIN32)
  target_link_libraries(p3express advapi32.lib ws2_32.lib)
endif()

if(ANDROID)
  target_link_libraries(p3express android)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3express
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3EXPRESS_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
