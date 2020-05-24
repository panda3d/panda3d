set(P3PGRAPH_HEADERS
  accumulatedAttribs.I accumulatedAttribs.h
  alphaTestAttrib.I alphaTestAttrib.h
  antialiasAttrib.I antialiasAttrib.h
  attribNodeRegistry.I attribNodeRegistry.h
  audioVolumeAttrib.I audioVolumeAttrib.h
  auxSceneData.I auxSceneData.h
  auxBitplaneAttrib.I auxBitplaneAttrib.h
  bamFile.I bamFile.h
  billboardEffect.I billboardEffect.h
  cacheStats.I cacheStats.h
  camera.I camera.h
  clipPlaneAttrib.I clipPlaneAttrib.h
  colorAttrib.I colorAttrib.h
  colorBlendAttrib.I colorBlendAttrib.h
  colorScaleAttrib.I colorScaleAttrib.h
  colorWriteAttrib.I colorWriteAttrib.h
  compassEffect.I compassEffect.h
  config_pgraph.h
  cullBin.I cullBin.h
  cullBinEnums.h
  cullBinAttrib.I cullBinAttrib.h
  cullBinManager.I cullBinManager.h
  cullFaceAttrib.I cullFaceAttrib.h
  cullHandler.I cullHandler.h
  cullPlanes.I cullPlanes.h
  cullResult.I cullResult.h
  cullTraverser.I cullTraverser.h
  cullTraverserData.I cullTraverserData.h
  cullableObject.I cullableObject.h
  decalEffect.I decalEffect.h
  depthOffsetAttrib.I depthOffsetAttrib.h
  depthTestAttrib.I depthTestAttrib.h
  depthWriteAttrib.I depthWriteAttrib.h
  findApproxLevelEntry.I findApproxLevelEntry.h
  findApproxPath.I findApproxPath.h
  fog.I fog.h
  fogAttrib.I fogAttrib.h
  geomDrawCallbackData.I geomDrawCallbackData.h
  geomNode.I geomNode.h
  geomTransformer.I geomTransformer.h
  internalNameCollection.I internalNameCollection.h
  lensNode.I lensNode.h
  light.I light.h
  lightAttrib.I lightAttrib.h
  lightRampAttrib.I lightRampAttrib.h
  loader.I loader.h
  loaderFileType.h
  loaderFileTypeBam.h
  loaderFileTypeRegistry.h
  logicOpAttrib.I logicOpAttrib.h
  materialAttrib.I materialAttrib.h
  materialCollection.I materialCollection.h
  modelFlattenRequest.I modelFlattenRequest.h
  modelLoadRequest.I modelLoadRequest.h
  modelSaveRequest.I modelSaveRequest.h
  modelNode.I modelNode.h
  modelPool.I modelPool.h
  modelRoot.I modelRoot.h
  nodePath.I nodePath.h
  nodePathCollection.I nodePathCollection.h
  nodePathComponent.I nodePathComponent.h
  occluderEffect.I occluderEffect.h
  occluderNode.I occluderNode.h
  pandaNode.I pandaNode.h
  pandaNodeChain.I pandaNodeChain.h
  planeNode.I planeNode.h
  paramNodePath.I paramNodePath.h
  polylightEffect.I polylightEffect.h
  polylightNode.I polylightNode.h
  portalNode.I portalNode.h
  portalClipper.I portalClipper.h
  renderAttrib.I renderAttrib.h
  renderAttribRegistry.I renderAttribRegistry.h
  renderEffect.I renderEffect.h
  renderEffects.I renderEffects.h
  renderModeAttrib.I renderModeAttrib.h
  renderState.I renderState.h
  rescaleNormalAttrib.I rescaleNormalAttrib.h
  sceneGraphReducer.I sceneGraphReducer.h
  sceneSetup.I sceneSetup.h
  scissorAttrib.I scissorAttrib.h
  scissorEffect.I scissorEffect.h
  shadeModelAttrib.I shadeModelAttrib.h
  shaderAttrib.I shaderAttrib.h
  shaderInput.I shaderInput.h
  shaderPool.I shaderPool.h
  showBoundsEffect.I showBoundsEffect.h
  stateMunger.I stateMunger.h
  stencilAttrib.I stencilAttrib.h
  texMatrixAttrib.I texMatrixAttrib.h
  texProjectorEffect.I texProjectorEffect.h
  textureAttrib.I textureAttrib.h
  texGenAttrib.I texGenAttrib.h
  textureStageCollection.I textureStageCollection.h
  transformState.I transformState.h
  transparencyAttrib.I transparencyAttrib.h
  weakNodePath.I weakNodePath.h
  workingNodePath.I workingNodePath.h
)

set(P3PGRAPH_SOURCES
  accumulatedAttribs.cxx
  alphaTestAttrib.cxx
  antialiasAttrib.cxx
  attribNodeRegistry.cxx
  audioVolumeAttrib.cxx
  auxBitplaneAttrib.cxx
  auxSceneData.cxx
  bamFile.cxx
  billboardEffect.cxx
  cacheStats.cxx
  camera.cxx
  clipPlaneAttrib.cxx
  colorAttrib.cxx
  colorBlendAttrib.cxx
  colorScaleAttrib.cxx
  colorWriteAttrib.cxx
  compassEffect.cxx
  config_pgraph.cxx
  cullBin.cxx
  cullBinAttrib.cxx
  cullBinManager.cxx
  cullFaceAttrib.cxx
  cullHandler.cxx
  cullPlanes.cxx
  cullResult.cxx
  cullTraverser.cxx
  cullTraverserData.cxx
  cullableObject.cxx
  decalEffect.cxx
  depthOffsetAttrib.cxx
  depthTestAttrib.cxx
  depthWriteAttrib.cxx
  findApproxLevelEntry.cxx
  findApproxPath.cxx
  fog.cxx
  fogAttrib.cxx
  geomDrawCallbackData.cxx
  geomNode.cxx
  geomTransformer.cxx
  internalNameCollection.cxx
  lensNode.cxx
  light.cxx
  lightAttrib.cxx
  lightRampAttrib.cxx
  loader.cxx
  loaderFileType.cxx
  loaderFileTypeBam.cxx
  loaderFileTypeRegistry.cxx
  logicOpAttrib.cxx
  materialAttrib.cxx
  materialCollection.cxx
  modelFlattenRequest.cxx
  modelLoadRequest.cxx
  modelSaveRequest.cxx
  modelNode.cxx
  modelPool.cxx
  modelRoot.cxx
  nodePath.cxx
  nodePathCollection.cxx
  nodePathComponent.cxx
  occluderEffect.cxx
  occluderNode.cxx
  pandaNode.cxx
  pandaNodeChain.cxx
  planeNode.cxx
  paramNodePath.cxx
  polylightEffect.cxx
  polylightNode.cxx
  portalNode.cxx
  portalClipper.cxx
  renderAttrib.cxx
  renderAttribRegistry.cxx
  renderEffect.cxx
  renderEffects.cxx
  renderModeAttrib.cxx
  renderState.cxx
  rescaleNormalAttrib.cxx
  sceneGraphReducer.cxx
  sceneSetup.cxx
  scissorAttrib.cxx
  scissorEffect.cxx
  shadeModelAttrib.cxx
  shaderAttrib.cxx
  shaderInput.cxx
  shaderPool.cxx
  showBoundsEffect.cxx
  stateMunger.cxx
  stencilAttrib.cxx
  texMatrixAttrib.cxx
  texProjectorEffect.cxx
  textureAttrib.cxx
  texGenAttrib.cxx
  textureStageCollection.cxx
  transformState.cxx
  transparencyAttrib.cxx
  weakNodePath.cxx
  workingNodePath.cxx
)

set(P3PGRAPH_IGATEEXT
  loaderFileTypeRegistry_ext.cxx
  loaderFileTypeRegistry_ext.h
  nodePathCollection_ext.cxx
  nodePathCollection_ext.h
  nodePath_ext.cxx
  nodePath_ext.h
  nodePath_ext.I
  pandaNode_ext.cxx
  pandaNode_ext.h
  pythonLoaderFileType.cxx
  pythonLoaderFileType.h
  renderState_ext.cxx
  renderState_ext.h
  shaderAttrib_ext.cxx
  shaderAttrib_ext.h
  shaderInput_ext.cxx
  shaderInput_ext.h
  transformState_ext.cxx
  transformState_ext.h
)

# This is a large file; let's build it separately
set_source_files_properties(nodePath.cxx PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

composite_sources(p3pgraph P3PGRAPH_SOURCES)
add_component_library(p3pgraph SYMBOL BUILDING_PANDA_PGRAPH
  ${P3PGRAPH_HEADERS} ${P3PGRAPH_SOURCES})
target_link_libraries(p3pgraph p3gobj p3event p3gsgbase p3putil p3linmath pandaexpress)
target_interrogate(p3pgraph ALL EXTENSIONS ${P3PGRAPH_IGATEEXT})

if(HAVE_EIGEN)
  # Eigen puts this over the threshold where bigobj is needed on MSVC
  set_target_properties(p3pgraph PROPERTIES MSVC_BIGOBJ ON)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3pgraph
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PGRAPH_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
