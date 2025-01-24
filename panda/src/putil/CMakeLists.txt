set(P3PUTIL_HEADERS
  animInterface.h animInterface.I
  autoTextureScale.h
  bam.h
  bamCache.h bamCache.I
  bamCacheIndex.h bamCacheIndex.I
  bamCacheRecord.h bamCacheRecord.I
  bamEnums.h
  bamReader.I bamReader.h bamReaderParam.I
  bamReaderParam.h
  bamWriter.I bamWriter.h
  bitArray.I bitArray.h
  bitMask.I bitMask.h
  buttonHandle.I buttonHandle.h
  buttonMap.I buttonMap.h
  buttonRegistry.I buttonRegistry.h
  cachedTypedWritableReferenceCount.h cachedTypedWritableReferenceCount.I
  callbackData.h callbackData.I
  callbackObject.h callbackObject.I
  clockObject.h clockObject.I
  collideMask.h
  colorSpace.h
  completable.I completable.h
  completionCounter.I completionCounter.h
  completionToken.I completionToken.h
  copyOnWriteObject.h copyOnWriteObject.I
  copyOnWritePointer.h copyOnWritePointer.I
  compareTo.I compareTo.h
  config_putil.N config_putil.h configurable.h
  cPointerCallbackObject.h cPointerCallbackObject.I
  datagramBuffer.I datagramBuffer.h
  datagramInputFile.I datagramInputFile.h
  datagramOutputFile.I datagramOutputFile.h
  doubleBitMask.I doubleBitMask.h
  drawMask.h
  factory.I factory.h
  factoryBase.I factoryBase.h
  factoryParam.I factoryParam.h factoryParams.I
  factoryParams.h
  firstOfPairCompare.I firstOfPairCompare.h
  firstOfPairLess.I firstOfPairLess.h
  gamepadButton.h
  globalPointerRegistry.I globalPointerRegistry.h
  indirectCompareNames.I indirectCompareNames.h
  indirectCompareSort.I indirectCompareSort.h
  indirectCompareTo.I indirectCompareTo.h
  ioPtaDatagramFloat.h ioPtaDatagramInt.h
  ioPtaDatagramShort.h keyboardButton.h
  iterator_types.h
  linkedListNode.I linkedListNode.h
  load_prc_file.h
  loaderOptions.I loaderOptions.h
  modifierButtons.I modifierButtons.h
  mouseButton.h
  nameUniquifier.I nameUniquifier.h
  nodeCachedReferenceCount.h nodeCachedReferenceCount.I
  paramValue.I paramValue.h
  pbitops.I pbitops.h
  pointerData.h pointerData.I
  portalMask.h
  pta_ushort.h
  simpleHashMap.I simpleHashMap.h
  sparseArray.I sparseArray.h
  timedCycle.I timedCycle.h typedWritable.I
  typedWritable.h typedWritableReferenceCount.I
  typedWritableReferenceCount.h updateSeq.I updateSeq.h
  uniqueIdAllocator.h
  vector_typedWritable.h
  vector_ulong.h
  vector_ushort.h vector_writable.h
  writableConfigurable.h
  writableParam.I writableParam.h
)

set(P3PUTIL_SOURCES
  animInterface.cxx
  autoTextureScale.cxx
  bamCache.cxx
  bamCacheIndex.cxx
  bamCacheRecord.cxx
  bamEnums.cxx
  bamReader.cxx bamReaderParam.cxx
  bamWriter.cxx
  bitArray.cxx
  bitMask.cxx
  buttonHandle.cxx buttonMap.cxx buttonRegistry.cxx
  cachedTypedWritableReferenceCount.cxx
  callbackData.cxx
  callbackObject.cxx
  clockObject.cxx
  colorSpace.cxx
  completionCounter.cxx
  copyOnWriteObject.cxx
  copyOnWritePointer.cxx
  config_putil.cxx configurable.cxx
  cPointerCallbackObject.cxx
  datagramBuffer.cxx
  datagramInputFile.cxx datagramOutputFile.cxx
  doubleBitMask.cxx
  factoryBase.cxx
  factoryParam.cxx factoryParams.cxx
  gamepadButton.cxx
  globalPointerRegistry.cxx
  ioPtaDatagramFloat.cxx
  ioPtaDatagramInt.cxx ioPtaDatagramShort.cxx
  keyboardButton.cxx
  linkedListNode.cxx
  load_prc_file.cxx
  loaderOptions.cxx
  modifierButtons.cxx
  mouseButton.cxx
  nameUniquifier.cxx
  nodeCachedReferenceCount.cxx
  paramValue.cxx
  pbitops.cxx
  pointerData.cxx
  pta_ushort.cxx
  simpleHashMap.cxx
  sparseArray.cxx
  timedCycle.cxx typedWritable.cxx
  typedWritableReferenceCount.cxx updateSeq.cxx
  uniqueIdAllocator.cxx
  vector_typedWritable.cxx
  vector_ulong.cxx
  vector_ushort.cxx vector_writable.cxx
  writableConfigurable.cxx writableParam.cxx
)

set(P3PUTIL_IGATEEXT
  bamReader_ext.cxx
  bamReader_ext.h
  bamWriter_ext.cxx
  bamWriter_ext.h
  bitArray_ext.cxx
  bitArray_ext.h
  bitMask_ext.h
  callbackObject_ext.h
  doubleBitMask_ext.h
  paramPyObject.cxx
  paramPyObject.h
  pythonCallbackObject.cxx
  pythonCallbackObject.h
  sparseArray_ext.cxx
  sparseArray_ext.h
  typedWritable_ext.cxx
  typedWritable_ext.h
)

composite_sources(p3putil P3PUTIL_SOURCES)
add_component_library(p3putil SYMBOL BUILDING_PANDA_PUTIL
  ${P3PUTIL_HEADERS} ${P3PUTIL_SOURCES})
target_link_libraries(p3putil p3linmath p3pipeline)
target_interrogate(p3putil ALL EXTENSIONS ${P3PUTIL_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3putil
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PUTIL_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
install(FILES config_putil.h COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
