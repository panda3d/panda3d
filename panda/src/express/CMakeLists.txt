set(P3EXPRESS_HEADERS
  buffer.I buffer.h
  ca_bundle_data_src.c
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
  profileTimer.I profileTimer.h
  pta_int.h
  pta_uchar.h pta_double.h pta_float.h
  pta_stdfloat.h
  ramfile.I ramfile.h
  referenceCount.I referenceCount.h
  subStream.I subStream.h subStreamBuf.h
  subfileInfo.h subfileInfo.I
  temporaryFile.h temporaryFile.I
  threadSafePointerTo.I threadSafePointerTo.h
  threadSafePointerToBase.I threadSafePointerToBase.h
  trueClock.I trueClock.h
  typedReferenceCount.I typedReferenceCount.h typedef.h
  vector_uchar.h vector_double.h vector_float.h
  vector_stdfloat.h
  virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h
  virtualFileComposite.h virtualFileComposite.I virtualFile.h
  virtualFileMount.I virtualFileMountMultifile.h
  virtualFileMountMultifile.I
  virtualFileMountRamdisk.h virtualFileMountRamdisk.I
  virtualFileMountSystem.h virtualFileMountSystem.I
  virtualFileSimple.h virtualFileSimple.I
  virtualFileSystem.h virtualFileSystem.I
  weakPointerCallback.I weakPointerCallback.h
  weakPointerTo.I weakPointerTo.h
  weakPointerToBase.I weakPointerToBase.h
  weakPointerToVoid.I weakPointerToVoid.h
  weakReferenceList.I weakReferenceList.h
  windowsRegistry.h
  zStream.I zStream.h zStreamBuf.h)

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
  profileTimer.cxx
  pta_int.cxx
  pta_uchar.cxx pta_double.cxx pta_float.cxx
  ramfile.cxx
  referenceCount.cxx
  subStream.cxx subStreamBuf.cxx
  subfileInfo.cxx
  temporaryFile.cxx
  threadSafePointerTo.cxx
  threadSafePointerToBase.cxx
  trueClock.cxx
  typedReferenceCount.cxx
  vector_uchar.cxx vector_double.cxx vector_float.cxx
  virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx
  virtualFileMount.cxx
  virtualFileMountMultifile.cxx
  virtualFileMountRamdisk.cxx
  virtualFileMountSystem.cxx
  virtualFileSimple.cxx virtualFileSystem.cxx
  weakPointerCallback.cxx
  weakPointerTo.cxx
  weakPointerToBase.cxx
  weakPointerToVoid.cxx
  weakReferenceList.cxx
  windowsRegistry.cxx
  zStream.cxx zStreamBuf.cxx)

set(P3EXPRESS_IGATEEXT
  memoryUsagePointers_ext.cxx
  memoryUsagePointers_ext.h
  ramfile_ext.cxx
  ramfile_ext.h
  streamReader_ext.cxx
  streamReader_ext.h
  virtualFileSystem_ext.cxx
  virtualFileSystem_ext.h)

composite_sources(p3express P3EXPRESS_SOURCES)

add_library(p3express ${P3EXPRESS_SOURCES} ${P3EXPRESS_HEADERS} ${P3EXPRESS_IGATEEXT})
target_link_libraries(p3express p3pandabase p3dtool p3dtoolconfig
  ${_TAR_LIBRARY})
target_interrogate(p3express ALL)

if(WIN32)
  target_link_libraries(p3express advapi32.lib ws2_32.lib)
endif()

install(TARGETS p3express DESTINATION lib)

#add_executable(p3expressTestTypes test_types.cxx)
#target_link_libraries(p3expressTestTypes p3express p3dtoolutil p3dtool p3prc p3dtoolconfig p3pystub)
#add_test(p3express_test_types p3expressTestTypes)

#add_executable(p3expressTestOrderedVector test_ordered_vector.cxx)
#target_link_libraries(p3expressTestOrderedVector p3putil p3dtoolutil p3dtool p3prc p3dtoolconfig p3pystub)
#add_test(p3express_test_ordered_vector p3expressTestOrderedVector)

if(HAVE_ZLIB)
  #add_executable(p3expressTestZstream test_zstream.cxx)
  #target_link_libraries(p3expressTestZstream p3express p3dtoolutil p3dtool p3prc p3dtoolconfig p3pystub)
  #add_test(p3express_test_zstream p3expressTestZstream)
endif()
