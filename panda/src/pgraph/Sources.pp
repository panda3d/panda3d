#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS lerp event gsgbase gobj putil linmath express pandabase

#begin lib_target
  #define TARGET pgraph
  
  #define SOURCES \
    ambientLight.I ambientLight.h \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    qpcamera.I qpcamera.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinBackToFront.I cullBinBackToFront.h \
    cullBinManager.I cullBinManager.h \
    cullBinUnsorted.I cullBinUnsorted.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullResult.I cullResult.h \
    qpcullTraverser.I qpcullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    directionalLight.I directionalLight.h \
    drawCullHandler.I drawCullHandler.h \
    qpfindApproxLevel.I qpfindApproxLevel.h \
    qpfindApproxLevelEntry.I qpfindApproxLevelEntry.h \
    qpfindApproxPath.I qpfindApproxPath.h \
    qpfog.I qpfog.h \
    fogAttrib.I fogAttrib.h \
    qpgeomNode.I qpgeomNode.h \
    qpgeomTransformer.I qpgeomTransformer.h \
    qplensNode.I qplensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    qplodNode.I qplodNode.h \
    materialAttrib.I materialAttrib.h \
    qpmodelNode.I qpmodelNode.h \
    qpmodelRoot.I qpmodelRoot.h \
    qpnodePath.I qpnodePath.h qpnodePath.cxx \
    qpnodePathCollection.I qpnodePathCollection.h \
    qpnodePathComponent.I qpnodePathComponent.h \
    qpnodePathLerps.h \
    pandaNode.I pandaNode.h \
    pointLight.I pointLight.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    sceneGraphAnalyzer.h \
    qpsceneGraphReducer.I qpsceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    selectiveChildNode.I selectiveChildNode.h \
    qpsequenceNode.I qpsequenceNode.h \
    showBoundsEffect.I showBoundsEffect.h \
    spotlight.I spotlight.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    textureCollection.I textureCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    workingNodePath.I workingNodePath.h

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  #define INCLUDED_SOURCES \
    ambientLight.cxx \
    billboardEffect.cxx \
    binCullHandler.cxx \
    qpcamera.cxx \
    colorAttrib.cxx \
    colorBlendAttrib.cxx \
    colorScaleAttrib.cxx \
    colorWriteAttrib.cxx \
    config_pgraph.cxx \
    cullBin.cxx \
    cullBinAttrib.cxx \
    cullBinBackToFront.cxx \
    cullBinManager.cxx \
    cullBinUnsorted.cxx \
    cullFaceAttrib.cxx \
    cullHandler.cxx \
    cullResult.cxx \
    qpcullTraverser.cxx \
    cullTraverserData.cxx \
    cullableObject.cxx \
    decalEffect.cxx \
    depthOffsetAttrib.cxx \
    depthTestAttrib.cxx \
    depthWriteAttrib.cxx \
    directionalLight.cxx \
    drawCullHandler.cxx \
    qpfindApproxLevel.cxx \
    qpfindApproxLevelEntry.cxx \
    qpfindApproxPath.cxx \
    qpfog.cxx \
    fogAttrib.cxx \
    qpgeomNode.cxx \
    qpgeomTransformer.cxx \
    qplensNode.cxx \
    light.cxx \
    lightAttrib.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    qplodNode.cxx \
    materialAttrib.cxx \
    qpmodelNode.cxx \
    qpmodelRoot.cxx \
    qpnodePath.cxx \
    qpnodePathCollection.cxx \
    qpnodePathComponent.cxx \
    qpnodePathLerps.cxx \
    pandaNode.cxx \
    pointLight.cxx \
    renderAttrib.cxx \
    renderEffect.cxx \
    renderEffects.cxx \
    renderModeAttrib.cxx \
    renderState.cxx \
    sceneGraphAnalyzer.cxx \
    qpsceneGraphReducer.cxx \
    sceneSetup.cxx \
    selectiveChildNode.cxx \
    qpsequenceNode.cxx \
    showBoundsEffect.cxx \
    spotlight.cxx \
    texMatrixAttrib.cxx \
    textureApplyAttrib.cxx \
    textureAttrib.cxx \
    textureCollection.cxx \
    transformState.cxx \
    transparencyAttrib.cxx \
    workingNodePath.cxx

  #if $[DONT_COMBINE_PGRAPH]    
    #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]
    #define INCLUDED_SOURCES
    #define COMBINED_SOURCES
  #endif

  #define INSTALL_HEADERS \
    ambientLight.I ambientLight.h \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    qpcamera.I qpcamera.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinBackToFront.I cullBinBackToFront.h \
    cullBinManager.I cullBinManager.h \
    cullBinUnsorted.I cullBinUnsorted.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullResult.I cullResult.h \
    qpcullTraverser.I qpcullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    directionalLight.I directionalLight.h \
    drawCullHandler.I drawCullHandler.h \
    qpfog.I qpfog.h \
    fogAttrib.I fogAttrib.h \
    qpgeomNode.I qpgeomNode.h \
    qpgeomTransformer.I qpgeomTransformer.h \
    qplensNode.I qplensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    qplodNode.I qplodNode.h \
    materialAttrib.I materialAttrib.h \
    qpmodelNode.I qpmodelNode.h \
    qpmodelRoot.I qpmodelRoot.h \
    qpnodePath.I qpnodePath.h \
    qpnodePathCollection.I qpnodePathCollection.h \
    qpnodePathComponent.I qpnodePathComponent.h \
    qpnodePathLerps.h \
    pandaNode.I pandaNode.h \
    pointLight.I pointLight.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    sceneGraphAnalyzer.h \
    qpsceneGraphReducer.I qpsceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    selectiveChildNode.I selectiveChildNode.h \
    qpsequenceNode.I qpsequenceNode.h \
    showBoundsEffect.I showBoundsEffect.h \
    spotlight.I spotlight.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    textureCollection.I textureCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    workingNodePath.I workingNodePath.h

// No need to install these.
//    qpfindApproxLevel.I qpfindApproxLevel.h \
//    qpfindApproxLevelEntry.I qpfindApproxLevelEntry.h \
//    qpfindApproxPath.I qpfindApproxPath.h \

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_pgraph

  #define SOURCES \
    test_pgraph.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pgraph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target
