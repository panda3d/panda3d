#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET graph
  #define LOCAL_LIBS \
    putil mathutil

  #define SOURCES \  
	allAttributesWrapper.I        nodeAttributes.cxx  \
	allAttributesWrapper.T        nodeAttributes.h  \
	allAttributesWrapper.cxx      nodeRelation.I  \
	allAttributesWrapper.h        nodeRelation.N  \
	allTransitionsWrapper.I       nodeRelation.T  \
	allTransitionsWrapper.T       nodeRelation.cxx  \
	allTransitionsWrapper.cxx     nodeRelation.h  \
	allTransitionsWrapper.h       nodeTransition.I  \
	arcChain.I                    nodeTransition.N  \
	arcChain.cxx                  nodeTransition.cxx  \
	arcChain.h                    nodeTransition.h  \
	bitMask32Transition.cxx       nodeTransitionCache.I  \
	bitMask32Transition.h         nodeTransitionCache.cxx  \
	bitMaskAttribute.T            nodeTransitionCache.h  \
	bitMaskAttribute.h            nodeTransitionCacheEntry.I  \
	bitMaskTransition.T           nodeTransitionCacheEntry.cxx  \
	bitMaskTransition.h           nodeTransitionCacheEntry.h  \
	boundedObject.I               nodeTransitionWrapper.I  \
	boundedObject.N               nodeTransitionWrapper.T  \
	boundedObject.cxx             nodeTransitionWrapper.cxx  \
	boundedObject.h               nodeTransitionWrapper.h  \
	config_graph.cxx              nodeTransitions.I  \
	config_graph.h                nodeTransitions.T  \
	dftraverser.T                 nodeTransitions.cxx  \
	dftraverser.h                 nodeTransitions.h  \
	graphReducer.cxx              nullAttributeWrapper.I  \
	graphReducer.h                nullAttributeWrapper.cxx  \
	immediateAttribute.cxx        nullAttributeWrapper.h  \
	immediateAttribute.h          nullLevelState.cxx  \
	immediateTransition.I         nullLevelState.h  \
	immediateTransition.cxx       nullTransitionWrapper.I  \
	immediateTransition.h         nullTransitionWrapper.cxx  \
	lmatrix4fTransition.cxx       nullTransitionWrapper.h  \
	lmatrix4fTransition.h         onAttribute.cxx  \
	matrixAttribute.T             onAttribute.h  \
	matrixAttribute.h             onOffAttribute.I  \
	matrixTransition.T            onOffAttribute.cxx  \
	matrixTransition.h            onOffAttribute.h  \
	multiAttribute.T              onOffTransition.I  \
	multiAttribute.h              onOffTransition.cxx  \
	multiNodeAttribute.cxx        onOffTransition.h  \
	multiNodeAttribute.h          onTransition.I  \
	multiNodeTransition.cxx       onTransition.cxx  \
	multiNodeTransition.h         onTransition.h  \
	multiTransition.T             pointerNameClass.h  \
	multiTransition.h             pt_NamedNode.N  \
	multiTransitionHelpers.I      pt_NamedNode.cxx  \
	multiTransitionHelpers.h      pt_NamedNode.h  \
	namedNode.I                   pt_Node.N  \
	namedNode.cxx                 pt_Node.cxx  \
	namedNode.h                   pt_Node.h  \
	node.I                        setTransitionHelpers.T  \
	node.cxx                      setTransitionHelpers.h  \
	node.h                          \
	nodeAttribute.I                 \
	nodeAttribute.N                 \
	nodeAttribute.cxx             transitionDirection.h  \
	nodeAttribute.h               traverserVisitor.T  \
	nodeAttributeWrapper.I        traverserVisitor.h  \
	nodeAttributeWrapper.T        vector_PT_Node.cxx  \
	nodeAttributeWrapper.cxx      vector_PT_Node.h  \
	nodeAttributeWrapper.h        wrt.I  \
	nodeAttributes.I              wrt.cxx  \
	nodeAttributes.N              wrt.h  \
	nodeAttributes.T  

  #define INSTALL_HEADERS \
	allAttributesWrapper.I      nodeAttributeWrapper.h  \
	allAttributesWrapper.T      nodeAttributes.I  \
	allAttributesWrapper.h      nodeAttributes.T  \
	allTransitionsWrapper.I     nodeAttributes.h  \
	allTransitionsWrapper.T     nodeRelation.I  \
	allTransitionsWrapper.h     nodeRelation.T  \
	arcChain.I                  nodeRelation.h  \
	arcChain.h                  nodeTransition.I  \
	bitMask32Transition.h       nodeTransition.h  \
	bitMaskAttribute.T          nodeTransitionCache.I  \
	bitMaskAttribute.h          nodeTransitionCache.h  \
	bitMaskTransition.T         nodeTransitionCacheEntry.I  \
	bitMaskTransition.h         nodeTransitionCacheEntry.h  \
	boundedObject.I             nodeTransitionWrapper.I  \
	boundedObject.h             nodeTransitionWrapper.T  \
	config_graph.h              nodeTransitionWrapper.h  \
	dftraverser.T               nodeTransitions.I  \
	dftraverser.h               nodeTransitions.T  \
	graphReducer.h              nodeTransitions.h  \
	immediateAttribute.h        nullAttributeWrapper.I  \
	immediateTransition.I       nullAttributeWrapper.h  \
	immediateTransition.h       nullLevelState.h  \
	lmatrix4fTransition.h       nullTransitionWrapper.I  \
	matrixAttribute.T           nullTransitionWrapper.h  \
	matrixAttribute.h           onAttribute.h  \
	matrixTransition.T          onOffAttribute.I  \
	matrixTransition.h          onOffAttribute.h  \
	multiAttribute.T            onOffTransition.I  \
	multiAttribute.h            onOffTransition.h  \
	multiNodeAttribute.h        onTransition.I  \
	multiNodeTransition.h       onTransition.h  \
	multiTransition.T           pointerNameClass.h  \
	multiTransition.h           pt_NamedNode.h  \
	multiTransitionHelpers.I    pt_Node.h  \
	multiTransitionHelpers.h    setTransitionHelpers.T  \
	namedNode.I                 setTransitionHelpers.h  \
	namedNode.h                 transitionDirection.h  \
	node.I                      traverserVisitor.T  \
	node.h                      traverserVisitor.h  \
	nodeAttribute.I             vector_PT_Node.h  \
	nodeAttribute.h             wrt.I  \
	nodeAttributeWrapper.I      wrt.h  \
	nodeAttributeWrapper.T

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

