#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET graph
  #define LOCAL_LIBS \
    putil mathutil

  #define SOURCES \
    allAttributesWrapper.I allAttributesWrapper.cxx \
    allAttributesWrapper.h allTransitionsWrapper.I \
    allTransitionsWrapper.cxx allTransitionsWrapper.h \
    arcChain.I arcChain.cxx arcChain.h \
    bitMask32Transition.cxx bitMask32Transition.h \
    bitMaskAttribute.I bitMaskAttribute.h \
    bitMaskTransition.I bitMaskTransition.h \
    boundedObject.I \
    boundedObject.N boundedObject.cxx boundedObject.h config_graph.cxx \
    config_graph.h graphReducer.cxx graphReducer.h immediateAttribute.I \
    immediateAttribute.cxx immediateAttribute.h immediateTransition.I \
    immediateTransition.cxx immediateTransition.h \
    lmatrix4fTransition.cxx lmatrix4fTransition.h \
    matrixAttribute.I matrixAttribute.h matrixTransition.I \
    matrixTransition.h \
    multiNodeAttribute.I \
    multiNodeAttribute.cxx multiNodeAttribute.h multiNodeTransition.I \
    multiNodeTransition.cxx multiNodeTransition.h namedNode.I \
    namedNode.cxx namedNode.h node.I node.cxx node.h nodeAttribute.I \
    nodeAttribute.N nodeAttribute.cxx nodeAttribute.h \
    nodeAttributeWrapper.I nodeAttributeWrapper.cxx \
    nodeAttributeWrapper.h nodeAttributes.I nodeAttributes.N \
    nodeAttributes.cxx nodeAttributes.h nodeRelation.I nodeRelation.N \
    nodeRelation.cxx nodeRelation.h nodeTransition.I nodeTransition.N \
    nodeTransition.cxx nodeTransition.h nodeTransitionCache.I \
    nodeTransitionCache.cxx nodeTransitionCache.h \
    nodeTransitionCacheEntry.I nodeTransitionCacheEntry.cxx \
    nodeTransitionCacheEntry.h nodeTransitionWrapper.I \
    nodeTransitionWrapper.cxx nodeTransitionWrapper.h nodeTransitions.I \
    nodeTransitions.cxx nodeTransitions.h nullAttributeWrapper.I \
    nullAttributeWrapper.cxx nullAttributeWrapper.h nullLevelState.cxx \
    nullLevelState.h nullTransitionWrapper.I nullTransitionWrapper.cxx \
    nullTransitionWrapper.h onAttribute.I onAttribute.cxx onAttribute.h \
    onOffAttribute.I onOffAttribute.cxx onOffAttribute.h \
    onOffTransition.I onOffTransition.cxx onOffTransition.h \
    onTransition.I onTransition.cxx onTransition.h pt_NamedNode.N \
    pt_NamedNode.cxx pt_NamedNode.h pt_Node.N pt_Node.cxx pt_Node.h \
    setTransitionHelpers.I setTransitionHelpers.h \
    traverserVisitor.I traverserVisitor.h \
    vector_PT_Node.cxx vector_PT_Node.h wrt.I wrt.h

  #define INSTALL_HEADERS \
    allAttributesWrapper.I allAttributesWrapper.h \
    allTransitionsWrapper.I allTransitionsWrapper.h \
    arcChain.I arcChain.h \
    bitMask32Transition.h bitMaskAttribute.I bitMaskAttribute.h \
    bitMaskTransition.I bitMaskTransition.h boundedObject.I \
    boundedObject.h config_graph.h dftraverser.I dftraverser.h \
    graphReducer.h immediateAttribute.I immediateAttribute.h \
    immediateTransition.I immediateTransition.h lmatrix4fTransition.h \
    matrixAttribute.I matrixAttribute.h matrixTransition.I \
    matrixTransition.h multiAttribute.I multiAttribute.h \
    multiNodeAttribute.I multiNodeAttribute.h multiNodeTransition.I \
    multiNodeTransition.h multiTransition.I multiTransition.h \
    multiTransitionHelpers.I multiTransitionHelpers.h namedNode.I \
    namedNode.h node.I node.h nodeAttribute.I nodeAttribute.h \
    nodeAttributeWrapper.I nodeAttributeWrapper.h nodeAttributes.I \
    nodeAttributes.h nodeRelation.I nodeRelation.h nodeTransition.I \
    nodeTransition.h nodeTransitionCache.I nodeTransitionCache.h \
    nodeTransitionCacheEntry.I nodeTransitionCacheEntry.h \
    nodeTransitionWrapper.I nodeTransitionWrapper.h nodeTransitions.I \
    nodeTransitions.h nullAttributeWrapper.I nullAttributeWrapper.h \
    nullLevelState.h nullTransitionWrapper.I nullTransitionWrapper.h \
    onAttribute.I onAttribute.h onOffAttribute.I onOffAttribute.h \
    onOffTransition.I onOffTransition.h onTransition.I onTransition.h \
    pointerNameClass.h pt_NamedNode.h pt_Node.h transitionDirection.h \
    traverserVisitor.I traverserVisitor.h vector_PT_Node.h wrt.I wrt.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_graph
  #define LOCAL_LIBS \
    graph putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graph.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_graphRead
  #define LOCAL_LIBS \
    putil graph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graphRead.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_graphWrite
  #define LOCAL_LIBS \
    graph putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graphWrite.cxx

#end test_bin_target

