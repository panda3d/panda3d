#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS event gsgbase gobj putil graph linmath express pandabase

#begin lib_target
  #define TARGET pgraph
  
  #define SOURCES \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    qpcamera.I qpcamera.h \
    colorAttrib.I colorAttrib.h \
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
    drawCullHandler.I drawCullHandler.h \
    qpfindApproxLevel.I qpfindApproxLevel.h \
    qpfindApproxLevelEntry.I qpfindApproxLevelEntry.h \
    qpfindApproxPath.I qpfindApproxPath.h \
    qpfog.I qpfog.h \
    fogAttrib.I fogAttrib.h \
    qpgeomNode.I qpgeomNode.h \
    qplensNode.I qplensNode.h \
    qplodNode.I qplodNode.h \
    materialAttrib.I materialAttrib.h \
    qpnodePath.I qpnodePath.h \
    qpnodePathCollection.I qpnodePathCollection.h \
    qpnodePathComponent.I qpnodePathComponent.h \
    pandaNode.I pandaNode.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    selectiveChildNode.I selectiveChildNode.h \
    qpsequenceNode.I qpsequenceNode.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
  #define INCLUDED_SOURCES \
    billboardEffect.cxx \
    binCullHandler.cxx \
    qpcamera.cxx \
    colorAttrib.cxx \
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
    drawCullHandler.cxx \
    qpfindApproxLevel.cxx \
    qpfindApproxLevelEntry.cxx \
    qpfindApproxPath.cxx \
    qpfog.cxx \
    fogAttrib.cxx \
    qpgeomNode.cxx \
    qplensNode.cxx \
    qplodNode.cxx \
    materialAttrib.cxx \
    qpnodePath.cxx \
    qpnodePathCollection.cxx \
    qpnodePathComponent.cxx \
    pandaNode.cxx \
    renderAttrib.cxx \
    renderEffect.cxx \
    renderEffects.cxx \
    renderModeAttrib.cxx \
    renderState.cxx \
    selectiveChildNode.cxx \
    qpsequenceNode.cxx \
    texMatrixAttrib.cxx \
    textureApplyAttrib.cxx \
    textureAttrib.cxx \
    transformState.cxx \
    transparencyAttrib.cxx

  #if $[DONT_COMBINE_PGRAPH]    
    #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]
    #define INCLUDED_SOURCES
    #define COMBINED_SOURCES
  #endif

  #define INSTALL_HEADERS \
    billboardEffect.I billboardEffect.h \
    binCullHandler.I binCullHandler.h \
    qpcamera.I qpcamera.h \
    colorAttrib.I colorAttrib.h \
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
    drawCullHandler.I drawCullHandler.h \
    qpfog.I qpfog.h \
    fogAttrib.I fogAttrib.h \
    qpgeomNode.I qpgeomNode.h \
    qplensNode.I qplensNode.h \
    qplodNode.I qplodNode.h \
    materialAttrib.I materialAttrib.h \
    qpnodePath.I qpnodePath.h \
    qpnodePathCollection.I qpnodePathCollection.h \
    qpnodePathComponent.I qpnodePathComponent.h \
    pandaNode.I pandaNode.h \
    renderAttrib.I renderAttrib.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    selectiveChildNode.I selectiveChildNode.h \
    qpsequenceNode.I qpsequenceNode.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    textureApplyAttrib.I textureApplyAttrib.h \
    textureAttrib.I textureAttrib.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h

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
