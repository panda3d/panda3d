#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS event gsgbase gobj putil graph linmath express pandabase

#begin lib_target
  #define TARGET pgraph
  
  #define SOURCES \
    billboardEffect.h billboardEffect.I \
    binCullHandler.h binCullHandler.I \
    qpcamera.h qpcamera.I \
    colorAttrib.h colorAttrib.I \
    colorWriteAttrib.h colorWriteAttrib.I \
    config_pgraph.h \
    cullBin.h cullBin.I \
    cullBinAttrib.h cullBinAttrib.I \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinManager.h cullBinManager.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    cullFaceAttrib.h cullFaceAttrib.I \
    cullHandler.h cullHandler.I \
    cullResult.h cullResult.I \
    qpcullTraverser.h qpcullTraverser.I \
    cullTraverserData.h cullTraverserData.I \
    cullableObject.h cullableObject.I \
    decalEffect.h decalEffect.I \
    depthTestAttrib.h depthTestAttrib.I \
    depthWriteAttrib.h depthWriteAttrib.I \
    drawCullHandler.h drawCullHandler.I \
    qpfindApproxLevel.I qpfindApproxLevel.h \
    qpfindApproxLevelEntry.I qpfindApproxLevelEntry.h \
    qpfindApproxPath.I qpfindApproxPath.h \
    qpgeomNode.h qpgeomNode.I \
    qplensNode.h qplensNode.I \
    qplodNode.h qplodNode.I \
    materialAttrib.h materialAttrib.I \
    qpnodePath.h qpnodePath.I \
    qpnodePathCollection.h qpnodePathCollection.I \
    qpnodePathComponent.h qpnodePathComponent.I \
    pandaNode.h pandaNode.I \
    renderAttrib.h renderAttrib.I \
    renderEffect.h renderEffect.I \
    renderEffects.h renderEffects.I \
    renderState.h renderState.I \
    selectiveChildNode.h selectiveChildNode.I \
    qpsequenceNode.h qpsequenceNode.I \
    textureApplyAttrib.h textureApplyAttrib.I \
    textureAttrib.h textureAttrib.I \
    transformState.h transformState.I \
    transparencyAttrib.h transparencyAttrib.I

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
  #define INCLUDED_SOURCES \
    billboardEffect.cxx \
    binCullHandler.cxx \
    qpcamera.cxx \
    colorAttrib.cxx \
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
    depthTestAttrib.cxx \
    depthWriteAttrib.cxx \
    drawCullHandler.cxx \
    qpfindApproxLevel.cxx \
    qpfindApproxLevelEntry.cxx \
    qpfindApproxPath.cxx \
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
    renderState.cxx \
    selectiveChildNode.cxx \
    qpsequenceNode.cxx \
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
    billboardEffect.h billboardEffect.I \
    binCullHandler.h binCullHandler.I \
    qpcamera.h qpcamera.I \
    colorAttrib.h colorAttrib.I \
    colorWriteAttrib.h colorWriteAttrib.I \
    config_pgraph.h \
    cullBin.h cullBin.I \
    cullBinAttrib.h cullBinAttrib.I \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinManager.h cullBinManager.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    cullFaceAttrib.h cullFaceAttrib.I \
    cullHandler.h cullHandler.I \
    cullResult.h cullResult.I \
    qpcullTraverser.h qpcullTraverser.I \
    cullTraverserData.h cullTraverserData.I \
    cullableObject.h cullableObject.I \
    decalEffect.h decalEffect.I \
    depthTestAttrib.h depthTestAttrib.I \
    depthWriteAttrib.h depthWriteAttrib.I \
    drawCullHandler.h drawCullHandler.I \
    qpgeomNode.h qpgeomNode.I \
    qplensNode.h qplensNode.I \
    qplodNode.h qplodNode.I \
    materialAttrib.h materialAttrib.I \
    qpnodePath.h qpnodePath.I \
    qpnodePathCollection.h qpnodePathCollection.I \
    qpnodePathComponent.h qpnodePathComponent.I \
    pandaNode.h pandaNode.I \
    renderAttrib.h renderAttrib.I \
    renderEffect.h renderEffect.I \
    renderEffects.h renderEffects.I \
    renderState.h renderState.I \
    selectiveChildNode.h selectiveChildNode.I \
    qpsequenceNode.h qpsequenceNode.I \
    textureApplyAttrib.h textureApplyAttrib.I \
    textureAttrib.h textureAttrib.I \
    transformState.h transformState.I \
    transparencyAttrib.h transparencyAttrib.I

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
