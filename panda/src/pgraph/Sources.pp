#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS event gsgbase gobj putil graph linmath express pandabase

#begin lib_target
  #define TARGET pgraph
  
  #define SOURCES \
    billboardAttrib.h billboardAttrib.I \
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
    cullableObject.h cullableObject.I \
    decalAttrib.h decalAttrib.I \
    depthTestAttrib.h depthTestAttrib.I \
    depthWriteAttrib.h depthWriteAttrib.I \
    drawCullHandler.h drawCullHandler.I \
    qpgeomNode.h qpgeomNode.I \
    qplensNode.h qplensNode.I \
    materialAttrib.h materialAttrib.I \
    nodeChain.h nodeChain.I \
    nodeChainComponent.h nodeChainComponent.I \
    pandaNode.h pandaNode.I \
    renderAttrib.h renderAttrib.I \
    renderState.h renderState.I \
    textureApplyAttrib.h textureApplyAttrib.I \
    textureAttrib.h textureAttrib.I \
    transformState.h transformState.I \
    transparencyAttrib.h transparencyAttrib.I

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
  #define INCLUDED_SOURCES \
    billboardAttrib.cxx \
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
    cullableObject.cxx \
    decalAttrib.cxx \
    depthTestAttrib.cxx \
    depthWriteAttrib.cxx \
    drawCullHandler.cxx \
    qpgeomNode.cxx \
    qplensNode.cxx \
    materialAttrib.cxx \
    nodeChain.cxx \
    nodeChainComponent.cxx \
    pandaNode.cxx \
    renderAttrib.cxx \
    renderState.cxx \
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
    billboardAttrib.h billboardAttrib.I \
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
    cullableObject.h cullableObject.I \
    decalAttrib.h decalAttrib.I \
    depthTestAttrib.h depthTestAttrib.I \
    depthWriteAttrib.h depthWriteAttrib.I \
    drawCullHandler.h drawCullHandler.I \
    qpgeomNode.h qpgeomNode.I \
    qplensNode.h qplensNode.I \
    materialAttrib.h materialAttrib.I \
    nodeChain.h nodeChain.I \
    nodeChainComponent.h nodeChainComponent.I \
    pandaNode.h pandaNode.I \
    renderAttrib.h renderAttrib.I \
    renderState.h renderState.I \
    textureApplyAttrib.h textureApplyAttrib.I \
    textureAttrib.h textureAttrib.I \
    transformState.h transformState.I \
    transparencyAttrib.h transparencyAttrib.I

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_pgraph

  #define SOURCES \
    test_pgraph.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pgraph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target
