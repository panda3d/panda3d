#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS \
    lerp event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient 
 

#begin lib_target
  #define TARGET pgraph

  #define SOURCES \
    accumulatedAttribs.I accumulatedAttribs.h \
    alphaTestAttrib.I alphaTestAttrib.h \  
    ambientLight.I ambientLight.h \
    antialiasAttrib.I antialiasAttrib.h \
    auxSceneData.I auxSceneData.h \
    bamFile.I bamFile.h \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    camera.I camera.h \
    clipPlaneAttrib.I clipPlaneAttrib.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    compassEffect.I compassEffect.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinBackToFront.I cullBinBackToFront.h \
    cullBinFixed.I cullBinFixed.h \
    cullBinFrontToBack.I cullBinFrontToBack.h \
    cullBinManager.I cullBinManager.h \
    cullBinUnsorted.I cullBinUnsorted.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullResult.I cullResult.h \
    cullTraverser.I cullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    directionalLight.I directionalLight.h \
    drawCullHandler.I drawCullHandler.h \
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    findApproxLevelEntry.I findApproxLevelEntry.h \
    findApproxPath.I findApproxPath.h \
    fog.I fog.h \
    fogAttrib.I fogAttrib.h \
    geomNode.I geomNode.h \
    geomTransformer.I geomTransformer.h \
    lensNode.I lensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    loader.I loader.h  \
    loaderFileType.h \
    loaderFileTypeBam.h  \
    loaderFileTypeRegistry.h \
    lodNode.I lodNode.h \
    materialAttrib.I materialAttrib.h \
    modelNode.I modelNode.h \
    modelPool.I modelPool.h \
    modelRoot.I modelRoot.h \
    nodePath.I nodePath.h nodePath.cxx \
    nodePathCollection.I nodePathCollection.h \
    nodePathComponent.I nodePathComponent.h \
    nodePathLerps.h \
    pandaNode.I pandaNode.h \
    planeNode.I planeNode.h \
    pointLight.I pointLight.h \
    polylightNode.I polylightNode.h \
    polylightEffect.I polylightEffect.h \
    portalNode.I portalNode.h \
    portalClipper.I portalClipper.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    rescaleNormalAttrib.I rescaleNormalAttrib.h \
    sceneGraphAnalyzer.h \
    sceneGraphReducer.I sceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    selectiveChildNode.I selectiveChildNode.h \
    sequenceNode.I sequenceNode.h \
    showBoundsEffect.I showBoundsEffect.h \
    spotlight.I spotlight.h \
    switchNode.I switchNode.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    texProjectorEffect.I texProjectorEffect.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    texGenAttrib.I texGenAttrib.h \
    textureCollection.I textureCollection.h \
    textureStageCollection.I textureStageCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    weakNodePath.I weakNodePath.h \
    workingNodePath.I workingNodePath.h

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  #define INCLUDED_SOURCES \
    accumulatedAttribs.cxx \
    alphaTestAttrib.cxx \  
    ambientLight.cxx \
    antialiasAttrib.cxx \
    auxSceneData.cxx \
    bamFile.cxx \
    billboardEffect.cxx \
    binCullHandler.cxx \
    camera.cxx \
    clipPlaneAttrib.cxx \
    colorAttrib.cxx \
    colorBlendAttrib.cxx \
    colorScaleAttrib.cxx \
    colorWriteAttrib.cxx \
    compassEffect.cxx \
    config_pgraph.cxx \
    cullBin.cxx \
    cullBinAttrib.cxx \
    cullBinBackToFront.cxx \
    cullBinFixed.cxx \
    cullBinFrontToBack.cxx \
    cullBinManager.cxx \
    cullBinUnsorted.cxx \
    cullFaceAttrib.cxx \
    cullHandler.cxx \
    cullResult.cxx \
    cullTraverser.cxx \
    cullTraverserData.cxx \
    cullableObject.cxx \
    decalEffect.cxx \
    depthOffsetAttrib.cxx \
    depthTestAttrib.cxx \
    depthWriteAttrib.cxx \
    directionalLight.cxx \
    drawCullHandler.cxx \
    fadeLodNode.cxx fadeLodNodeData.cxx \
    findApproxLevelEntry.cxx \
    findApproxPath.cxx \
    fog.cxx \
    fogAttrib.cxx \
    geomNode.cxx \
    geomTransformer.cxx \
    lensNode.cxx \
    light.cxx \
    lightAttrib.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    loader.cxx \
    loaderFileType.cxx  \
    loaderFileTypeBam.cxx \
    loaderFileTypeRegistry.cxx  \
    lodNode.cxx \
    materialAttrib.cxx \
    modelNode.cxx \
    modelPool.cxx \
    modelRoot.cxx \
    nodePathCollection.cxx \
    nodePathComponent.cxx \
    nodePathLerps.cxx \
    pandaNode.cxx \
    planeNode.cxx \
    pointLight.cxx \
    polylightNode.cxx \
    polylightEffect.cxx \
    portalNode.cxx \
    portalClipper.cxx \
    renderAttrib.cxx \
    renderEffect.cxx \
    renderEffects.cxx \
    renderModeAttrib.cxx \
    renderState.cxx \
    rescaleNormalAttrib.cxx \
    sceneGraphAnalyzer.cxx \
    sceneGraphReducer.cxx \
    sceneSetup.cxx \
    selectiveChildNode.cxx \
    sequenceNode.cxx \
    showBoundsEffect.cxx \
    spotlight.cxx \
    switchNode.cxx \
    texMatrixAttrib.cxx \
    texProjectorEffect.cxx \
    textureApplyAttrib.cxx \
    textureAttrib.cxx \
    texGenAttrib.cxx \
    textureCollection.cxx \
    textureStageCollection.cxx \
    transformState.cxx \
    transparencyAttrib.cxx \
    weakNodePath.cxx \
    workingNodePath.cxx

  #define INSTALL_HEADERS \
    accumulatedAttribs.I accumulatedAttribs.h \
    alphaTestAttrib.I alphaTestAttrib.h \  
    ambientLight.I ambientLight.h \
    antialiasAttrib.I antialiasAttrib.h \
    auxSceneData.I auxSceneData.h \
    bamFile.I bamFile.h \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    camera.I camera.h \
    clipPlaneAttrib.I clipPlaneAttrib.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    compassEffect.I compassEffect.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinBackToFront.I cullBinBackToFront.h \
    cullBinFixed.I cullBinFixed.h \
    cullBinFrontToBack.I cullBinFrontToBack.h \
    cullBinManager.I cullBinManager.h \
    cullBinUnsorted.I cullBinUnsorted.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullResult.I cullResult.h \
    cullTraverser.I cullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    directionalLight.I directionalLight.h \
    drawCullHandler.I drawCullHandler.h \
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    fog.I fog.h \
    fogAttrib.I fogAttrib.h \
    geomNode.I geomNode.h \
    geomTransformer.I geomTransformer.h \
    lensNode.I lensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    loader.I loader.h \
    loaderFileType.h \
    loaderFileTypeBam.h \
    loaderFileTypeRegistry.h \
    lodNode.I lodNode.h \
    materialAttrib.I materialAttrib.h \
    modelNode.I modelNode.h \
    modelPool.I modelPool.h \
    modelRoot.I modelRoot.h \
    nodePath.I nodePath.h \
    nodePathCollection.I nodePathCollection.h \
    nodePathComponent.I nodePathComponent.h \
    nodePathLerps.h \
    pandaNode.I pandaNode.h \
    planeNode.I planeNode.h \
    pointLight.I pointLight.h \
    polylightNode.I polylightNode.h \
    polylightEffect.I polylightEffect.h \
    portalNode.I portalNode.h \
    portalClipper.I portalClipper.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    rescaleNormalAttrib.I rescaleNormalAttrib.h \
    sceneGraphAnalyzer.h \
    sceneGraphReducer.I sceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    selectiveChildNode.I selectiveChildNode.h \
    sequenceNode.I sequenceNode.h \
    showBoundsEffect.I showBoundsEffect.h \
    spotlight.I spotlight.h \
    switchNode.I switchNode.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    texProjectorEffect.I texProjectorEffect.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    texGenAttrib.I texGenAttrib.h \
    textureCollection.I textureCollection.h \
    textureStageCollection.I textureStageCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    weakNodePath.I weakNodePath.h \
    workingNodePath.I workingNodePath.h

// No need to install these.
//    findApproxLevelEntry.I findApproxLevelEntry.h \
//    findApproxPath.I findApproxPath.h \

  #define IGATESCAN all

// Uncomment these lines to compile everything individually instead of
// combining into pgraph_composite*.cxx.

//  #define COMBINED_SOURCES
//  #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]

#end lib_target


#begin test_bin_target
  #define TARGET test_pgraph

  #define SOURCES \
    test_pgraph.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pgraph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target
